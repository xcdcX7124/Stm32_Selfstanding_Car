#ifndef __ENCODER_H
#define __ENCODER_H

void Encoder_Init(void);
int16_t GetLeftEncoder(void);
int16_t GetRightEncoder(void);
float GetLeftEncoder_Speed(void);
float GetRightEncoder_Speed(void);
//int16_t Encoder_GetLeft(void);
//void EXTI0_IRQHandler(void);
//void EXTI1_IRQHandler(void);
//int16_t Encoder_GetRight(void);
//void EXTI6_IRQHandler(void);
//void EXTI7_IRQHandler(void);  外部中断部分的函数


#endif
