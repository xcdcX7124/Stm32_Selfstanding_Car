#include <stdio.h>
#include <String.h>
#include "BLEtoJDY.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#define BLE_Target_Name  "JDY-08"

// 全局句柄变量
static uint16_t g_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t g_tx_chr_val_handle = 0; // 自动解析出来的 0xFFE1 特征真实 Handle

// 函数前置声明
static void start_scan(void);
static int ble_central_gap_event(struct ble_gap_event *event, void *arg);


// ==================== 1. 特征发现与服务发现回调 ====================

/**
 * @brief 步骤 2: 找到服务后，寻找其内部特征的回调函数
 */
static int on_disc_chr(uint16_t conn_handle,
                       const struct ble_gatt_error *error,
                       const struct ble_gatt_chr *chr,
                       void *arg)
{
    if (error->status == 0 && chr != NULL) {
        uint16_t uuid16 = ble_uuid_u16(&chr->uuid.u);
        printf("[GATT Disc] Found Characteristic UUID: 0x%04X, Handle: %d\n", uuid16, chr->val_handle);

        // 匹配 JDY-33 的串口数据写入特征 0xFFE1
        if (uuid16 == 0xFFE1) {
            g_tx_chr_val_handle = chr->val_handle;
            printf("\n==========================================\n");
            printf(" SUCCESS: Found JDY-08 Tx Handle: %d\n", g_tx_chr_val_handle);
            printf("==========================================\n\n");
        }
        return 0;
    }
    
    if (error->status == BLE_HS_EDONE) {
        printf("[GATT Disc] Characteristic discovery complete.\n");
    } else if (error->status != 0) {
        printf("[GATT Disc] Characteristic error, status=%d\n", error->status);
    }
    return 0;
}

/**
 * @brief 步骤 1: 查找主服务 0xFFE0 的回调函数
 */
static int on_disc_svc(uint16_t conn_handle,
                       const struct ble_gatt_error *error,
                       const struct ble_gatt_svc *service,
                       void *arg)
{
    if (error->status == 0 && service != NULL) {
        uint16_t uuid16 = ble_uuid_u16(&service->uuid.u);
        printf("[GATT Disc] Found Service UUID: 0x%04X (Handle %d to %d)\n",
               uuid16, service->start_handle, service->end_handle);

        // 找到 JDY 的主服务 0xFFE0，立即去查这个服务范围内的特征
        if (uuid16 == 0xFFE0) {
            printf("[GATT Disc] Finding characteristics for Service 0xFFE0...\n");
            ble_gattc_disc_all_chrs(conn_handle,
                                    service->start_handle,
                                    service->end_handle,
                                    on_disc_chr,
                                    NULL);
        }
        return 0;
    }

    if (error->status == BLE_HS_EDONE) {
        printf("[GATT Disc] Service discovery complete.\n");
    } else if (error->status != 0) {
        printf("[GATT Disc] Service discovery error, status=%d\n", error->status);
    }
    return 0;
}


// ==================== 2. GAP 事件回调（扫描与连接处理） ====================

static int ble_central_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_hs_adv_fields fields;

    switch (event->type) {
    // 扫描到广播数据包
    case BLE_GAP_EVENT_DISC:
        // 解析广播包中的字段
        if (ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data) == 0) {
            if (fields.name != NULL && fields.name_len > 0) {
                // 打印扫描到的设备名称
                printf("Scanned device: %.*s\n", fields.name_len, fields.name);

                // 匹配设备名称
                if (strncmp((char *)fields.name, BLE_Target_Name, fields.name_len) == 0) {
                    printf("Target device found! Stop scanning and connect...\n");
                    
                    // 1. 停止扫描
                    ble_gap_disc_cancel();

                    // 2. 发起连接
                    int rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr,
                                             30000, NULL, ble_central_gap_event, NULL);
                    if (rc != 0) {
                        printf("Failed to send connect request, rc=%d\n", rc);
                        start_scan(); // 失败则重新扫描
                    }
                }
            }
        }
        break;

    // 连接成功/失败事件
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            printf("Connected to target device successfully!\n");
            g_conn_handle = event->connect.conn_handle;

            // 连接建立成功后，开始查询所有服务
            int rc = ble_gattc_disc_all_svcs(g_conn_handle, on_disc_svc, NULL);
            if (rc != 0) {
                printf("Failed to discover services, rc=%d\n", rc);
            }
        } else {
            printf("Connection failed, status=%d. Restarting scan...\n", event->connect.status);
            g_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            start_scan();
        }
        break;

    // 连接断开事件
    case BLE_GAP_EVENT_DISCONNECT:
        printf("Disconnected from device, reason=%d. Restarting scan...\n", event->disconnect.reason);
        g_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        g_tx_chr_val_handle = 0; // 重置句柄
        start_scan();
        break;

    default:
        break;
    }

    return 0;
}


// ==================== 3. 启动扫描逻辑 ====================

static void start_scan(void)
{
    struct ble_gap_disc_params disc_params = {
        .filter_duplicates = 1,
        .passive = 0, // 主动扫描以获取设备完整名称
        .itvl = BLE_GAP_SCAN_ITVL_MS(100),
        .window = BLE_GAP_SCAN_WIN_MS(50),
    };

    printf("Starting BLE scan...\n");
    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params,
                          ble_central_gap_event, NULL);
    if (rc != 0) {
        printf("Failed to start scan, rc=%d\n", rc);
    }
}


// ==================== 4. 外部调用的数据发送接口 ====================

int BLE_Send_Data(const uint8_t *data, uint16_t len)
{
    // 检查连接状态和特征句柄是否成功解析出来
    if (g_conn_handle == BLE_HS_CONN_HANDLE_NONE || g_tx_chr_val_handle == 0) {
        printf("Send failed: Device not connected or Tx Handle not discovered yet.\n");
        return -1;
    }

    // 向自动解析到的真实 Handle 发送数据
    int rc = ble_gattc_write_no_rsp_flat(g_conn_handle, g_tx_chr_val_handle, data, len);
    return rc;
}

// ==================== 5. 初始化与同步逻辑 ====================

static void on_sync(void)
{
    start_scan(); // 协议栈准备就绪后自动启动扫描
}

void host_task(void *arg)
{
    nimble_port_run();
}

void BLE_Init(void)
{
    nvs_flash_init();
    nimble_port_init();

    ble_hs_cfg.sm_bonding = 0;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 0;

    ble_att_set_preferred_mtu(23);

    ble_hs_cfg.sync_cb = on_sync;

    nimble_port_freertos_init(host_task);
}

void BLE_Send_Joystick_Packet(int LeftHorizon,int LeftVertical,int RightHorizon,int RightVertical){

    char Package_Cache[64];
    int Package_Length = snprintf(Package_Cache,sizeof(Package_Cache),"[joy,%d,%d,%d,%d]",LeftHorizon,LeftVertical,RightHorizon,RightVertical);
    BLE_Send_Data((const uint8_t *)Package_Cache,(uint16_t)Package_Length);

}

void BLE_Send_Key_Packet(int Key1,int Key2,int Key3,int Key4){

    char Package_Cache[64];
    int Package_Length = snprintf(Package_Cache,sizeof(Package_Cache),"[key,%d,%d,%d,%d]",Key1,Key2,Key3,Key4);
    BLE_Send_Data((const uint8_t *)Package_Cache,(uint16_t)Package_Length);

}
