#ifndef __PWMA_H
#define __PWMA_H

void PWMA_Init(void);
void PWMA_SetLeftForward(void);
void PWMA_SetLeftBackward(void);
void PWMA_SetRightForward(void);
void PWMA_SetRightBackward(void);
void PWMA_SetLeftPWM(int16_t PWM_Value);
void PWMA_SetRightPWM(int16_t PWM_Value);

#endif
