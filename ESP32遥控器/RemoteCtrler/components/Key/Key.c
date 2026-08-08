#include <stdio.h>
#include "Key.h"
#include "driver/gpio.h"
#include "esp_timer.h"

uint8_t KEY1_Status, KEY2_Status, KEY3_Status, KEY4_Status; // 状态0为空闲，状态1为消抖，状态2为判断短按，状态3为判断长按
uint8_t Key_Mode_Status = 0;                                // 模式0为菜单模式 ，模式1为摇杆遥控模式， 模式2为按键遥控模式，长按按键来控制小车
uint8_t KEY1_Counter, KEY2_Counter, KEY3_Counter, KEY4_Counter;

void Key_UpdateStatus(uint8_t Key_Num)
{
    int Key_Level = 0;
    switch (Key_Num)
    {
    case 1:
        Key_Level = !gpio_get_level(GPIO_NUM_1);
        if (KEY1_Status == 0 && Key_Level == 1)
            KEY1_Status++;
        else if (KEY1_Status == 1 && Key_Level == 1)
            KEY1_Status++;
        else if ((KEY1_Status == 1 || KEY1_Status == 2 || KEY1_Status == 3) && Key_Level == 0)
        {
            KEY1_Status = 0;
            KEY1_Counter = 0;
        }
        if (KEY1_Status == 2)
            KEY1_Counter++;
        if (KEY1_Status == 2 && KEY1_Counter >= 2)
            KEY1_Status++;
        break;
    case 2:
        Key_Level = !gpio_get_level(GPIO_NUM_2);
        if (KEY2_Status == 0 && Key_Level == 1)
            KEY2_Status++;
        else if (KEY2_Status == 1 && Key_Level == 1)
            KEY2_Status++;
        else if ((KEY2_Status == 1 || KEY2_Status == 2 || KEY2_Status == 3) && Key_Level == 0)
        {
            KEY2_Status = 0;
            KEY2_Counter = 0;
        }
        if (KEY2_Status == 2)
            KEY2_Counter++;
        if (KEY2_Status == 2 && KEY2_Counter >= 2)
            KEY2_Status++;
        break;
    case 3:
        Key_Level = !gpio_get_level(GPIO_NUM_38);
        if (KEY3_Status == 0 && Key_Level == 1)
            KEY3_Status++;
        else if (KEY3_Status == 1 && Key_Level == 1)
            KEY3_Status++;
        else if ((KEY3_Status == 1 || KEY3_Status == 2 || KEY3_Status == 3) && Key_Level == 0)
        {
            KEY3_Status = 0;
            KEY3_Counter = 0;
        }
        if (KEY3_Status == 2)
            KEY3_Counter++;
        if (KEY3_Status == 2 && KEY3_Counter >= 2)
            KEY3_Status++;
        break;
    case 4:
        Key_Level = !gpio_get_level(GPIO_NUM_21);
        if (KEY4_Status == 0 && Key_Level == 1)
            KEY4_Status++;
        else if (KEY4_Status == 1 && Key_Level == 1)
            KEY4_Status++;
        else if ((KEY4_Status == 1 || KEY4_Status == 2 || KEY4_Status == 3) && Key_Level == 0)
        {
            KEY4_Status = 0;
            KEY4_Counter = 0;
        }
        if (KEY4_Status == 2)
            KEY4_Counter++;
        if (KEY4_Status == 2 && KEY4_Counter >= 2)
            KEY4_Status++;
        break;
    }
}

void Key_Init(void)
{
    gpio_config_t GPIO_InitStucture = {
        // 设置4个按钮引脚为上拉输入
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_1) | (1ULL << GPIO_NUM_2) | (1ULL << GPIO_NUM_38) | (1ULL << GPIO_NUM_21),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&GPIO_InitStucture);

}
