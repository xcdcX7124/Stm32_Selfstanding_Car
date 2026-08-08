#ifndef __KEY_H_
#define __KEY_H_

// extern uint8_t KEY1_Flag,KEY2_Flag,KEY3_Flag,KEY4_Flag;
extern uint8_t KEY1_Status,KEY2_Status,KEY3_Status,KEY4_Status;
extern uint8_t Key_Mode_Status;

void Key_UpdateStatus(uint8_t Key_Num);
void Key_Init(void);
// void GPIO_Pin_46_EXTI_Handler(void *param);
// void GPIO_Pin_2_EXTI_Handler(void *param);
// void GPIO_Pin_38_EXTI_Handler(void *param);
// void GPIO_Pin_21_EXTI_Handler(void *param);


#endif
