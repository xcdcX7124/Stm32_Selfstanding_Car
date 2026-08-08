#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Key.h"
#include "OLED.h"
#include "JoyStick.h"
#include "BLEtoJDY.h"
#include "esp_timer.h"

uint64_t Timer_Last, Timer_Now;
uint16_t KeyCtrl_ExitTimer = 0;
uint8_t KeyCtrl_ExitTimerStart = 0, Ctrl_Startup = 0, KeyCtrl_Release_Flag = 1; // KeyCtrl_Release_Flag是用于检测进入按键遥控模式以后松手一次才能产生控制信号防止误操作
int Ctrl_Back_Forward, Ctrl_Steering;

void app_main(void)
{
    Key_Init();
    OLED_Init();
    OLED_Clear();
    JoyStick_Init();
    BLE_Init();

    Timer_Last = esp_timer_get_time();

    Key_Mode_Status = 0;

    BLE_Send_Key_Packet(0, 0, 0, 0);
    BLE_Send_Joystick_Packet(0, 0, 0, 0);

    while (1)
    {
        Timer_Now = esp_timer_get_time();
        if (Timer_Now - Timer_Last >= 5000)
        { // 每5ms更新一次按钮状态
            Timer_Last = Timer_Now;
            for (uint8_t k = 0; k <= 4; k++)
            {
                Key_UpdateStatus(k);
            }
            if (KEY1_Status == 2 && Key_Mode_Status != 2)
            { // 按键1确认按下、不是按键遥控模式  则进入下一个模式
                Key_Mode_Status++;
            }

            if (Key_Mode_Status == 0)
            { // 菜单模式  数据包格式：[key,0,0,0,0]
                if (KEY3_Status == 2)
                {
                    if (KeyCtrl_ExitTimerStart == 1)
                    {
                        KeyCtrl_ExitTimerStart = 0;
                        Ctrl_Startup = !Ctrl_Startup;
                    }
                    else
                        KeyCtrl_ExitTimerStart = 1;
                }
                else
                {
                    if (KeyCtrl_ExitTimer <= 5 && KeyCtrl_ExitTimerStart == 1)
                        KeyCtrl_ExitTimer++;
                    else
                    {
                        KeyCtrl_ExitTimerStart = 0;
                        KeyCtrl_ExitTimer = 0;
                    }
                }
                BLE_Send_Key_Packet(Ctrl_Startup, 0, 0, 0);

                OLED_Clear();
                OLED_ShowString(15, 0, "ESP32 Ctrler", OLED_8X16);
                OLED_ShowString(0, 16 * 3, "CarPower:", OLED_8X16);
                if (Ctrl_Startup == 0)
                    OLED_ShowString(8 * 10, 16 * 3, "OFF", OLED_8X16);
                else
                    OLED_ShowString(8 * 10, 16 * 3, "ON", OLED_8X16);
                OLED_Update();
            }

            else if (Key_Mode_Status == 1)
            { // 模式1 摇杆遥控模式 数据包格式：[joystick,0,0,0,0]

                if (KEY3_Status == 2)
                    Key_Mode_Status = 0; // 按下按键3回到模式0

                BLE_Send_Joystick_Packet(JoyStick_GetLeftHorizon(), JoyStick_GetLeftVertical(), JoyStick_GetRightHorizon(), JoyStick_GetRightVertical());
                vTaskDelay(pdMS_TO_TICKS(10));

                OLED_Clear();
                OLED_ShowString(11, 0, "JoyStick Ctrl", OLED_8X16);
                OLED_ShowString(1, 16 * 2, "Power:", OLED_8X16);
                OLED_ShowString(1, 16 * 3, "Steering:", OLED_8X16);
                OLED_ShowSignedNum(8 * 7, 16 * 2, JoyStick_GetLeftVertical(), 3, OLED_8X16);
                OLED_ShowSignedNum(8 * 10, 16 * 3, JoyStick_GetRightHorizon(), 3, OLED_8X16);
                OLED_Update();
            }

            else if (Key_Mode_Status == 2)
            { // 模式2 按键遥控模式  数据包格式：[joystick,0,0,0,0]
                if (KEY1_Status == 0)
                    KeyCtrl_Release_Flag = 0; // 松开按键1后标志位归零允许输出指令，防止误控制
                if (KeyCtrl_Release_Flag == 0)
                {
                    if (JoyStick_GetLeftHorizon() <= -95 && JoyStick_GetRightVertical() >= 95) // 左摇杆向左，右摇杆向上保持3秒  则退出模式2回到模式0，中途松手重置定时
                    {
                        KeyCtrl_ExitTimerStart = 1;
                        if (KeyCtrl_ExitTimer <= 40)
                            KeyCtrl_ExitTimer++;
                        else
                        {
                            KeyCtrl_ExitTimer = 0;
                            KeyCtrl_ExitTimerStart = 0;
                            Key_Mode_Status = 0;
                            KeyCtrl_Release_Flag = 1;
                        }
                    }
                    else
                    {
                        if (KeyCtrl_ExitTimerStart == 1)
                        {
                            KeyCtrl_ExitTimer = 0;
                            KeyCtrl_ExitTimerStart = 0;
                        }
                    }

                    /* 此处要写按键遥控的映射，比如按Key1键映射输出左摇杆垂直轴输出+100 */
                    if (KEY1_Status == 3 && KEY2_Status == 0)
                        Ctrl_Back_Forward = 100;
                    else if (KEY1_Status == 0 && KEY2_Status == 3)
                        Ctrl_Back_Forward = -100;
                    else
                        Ctrl_Back_Forward = 0;

                    if (KEY3_Status == 3 && KEY4_Status == 0)
                        Ctrl_Steering = -100;
                    else if (KEY3_Status == 0 && KEY4_Status == 3)
                        Ctrl_Steering = 100;
                    else
                        Ctrl_Steering = 0;

                    BLE_Send_Joystick_Packet(0, Ctrl_Back_Forward, Ctrl_Steering, 0);

                    OLED_Clear();
                    OLED_ShowString(31, 0, "Key Ctrl", OLED_8X16);
                    OLED_ShowString(1, 16 * 1, "Power:", OLED_8X16);
                    OLED_ShowString(1, 16 * 2, "Steering:", OLED_8X16);
                    OLED_ShowSignedNum(8 * 7, 16 * 1, Ctrl_Back_Forward, 3, OLED_8X16);
                    OLED_ShowSignedNum(8 * 10, 16 * 2, Ctrl_Steering, 3, OLED_8X16);
                    OLED_ShowString(0, 55, "LS Left RS Up to Exit", OLED_6X8);
                    OLED_Update();
                }
            }
        }

        vTaskDelay(1);
    }
}