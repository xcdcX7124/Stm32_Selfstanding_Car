#include "stm32f10x.h"                  // Device header

void PWMA_Init(void){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;//配置PA8和PA11为复用推挽输出
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //配置PA8
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
		GPIO_Init(GPIOA,&GPIO_InitStructure);   //配置PA11

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);	  //TIM1时钟使能

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure); //给结构体赋一个初始值
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;  //不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;  //向上计数
	TIM_TimeBaseInitStructure.TIM_Period=100 - 1; //ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler=7200 - 1;  //PSC
	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure); ///配置TIM1
	
	TIM_OCInitTypeDef TIM_OCInitStructure; //配置输出比较（OC）
	TIM_OCStructInit(&TIM_OCInitStructure); //给结构体赋一个初始值
	
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=0;  //占空比
	
	TIM_OC1Init(TIM1,&TIM_OCInitStructure);  //配置OC1和OC4
	TIM_OC4Init(TIM1,&TIM_OCInitStructure);  

	TIM_CtrlPWMOutputs(TIM1, ENABLE);  //开启PWM输出（TIM1高级定时器才有，TIM2/3/4都不需要这一行）
	TIM_Cmd(TIM1,ENABLE);  //TIM1定时器使能
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;    //设置正反转控制引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;  
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
}

//AIN1 --> PB13
//AIN2 --> PB12
//BIN1 --> PB14
//BIN2 --> PB15

void PWMA_SetLeftForward(void){
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_RESET);     //AIN1
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);   //AIN2
}

void PWMA_SetLeftBackward(void){
	GPIO_WriteBit(GPIOB,GPIO_Pin_13,Bit_SET); //AIN1
	GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);   //AIN2
}

void PWMA_SetRightForward(void){
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_RESET);     //BIN1
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_SET);   //BIN2
}

void PWMA_SetRightBackward(void){
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET); //BIN1
	GPIO_WriteBit(GPIOB,GPIO_Pin_15,Bit_RESET);   //BIN2
}

void PWMA_SetLeftPWM(int16_t PWM_Value){
	if (PWM_Value >=0) {
		PWMA_SetLeftForward();
		TIM_SetCompare4(TIM1,PWM_Value);

	}
	if (PWM_Value < 0) {
		PWMA_SetLeftBackward();
		TIM_SetCompare4(TIM1,(PWM_Value-(2 * PWM_Value)));
}
}


void PWMA_SetRightPWM(int16_t PWM_Value){
	if (PWM_Value >=0) {
		PWMA_SetRightForward();
		TIM_SetCompare1(TIM1,PWM_Value);

	}
	if (PWM_Value < 0) {
		PWMA_SetRightBackward();
		TIM_SetCompare1(TIM1,(PWM_Value-(2 * PWM_Value)));
	}
}

