#include "stm32f10x.h"                  // Device header

int16_t EncoderL,EncoderR;

void Encoder_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStrcture;
	GPIO_InitStrcture.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStrcture.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStrcture.GPIO_Speed=GPIO_Speed_50MHz;   //将PA0和PA1设置为上拉输入
	GPIO_Init(GPIOA,&GPIO_InitStrcture);
	GPIO_InitStrcture.GPIO_Pin=GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitStrcture);

	GPIO_InitStrcture.GPIO_Pin=GPIO_Pin_6;   //将PB6和PB7设置为上拉输入
	GPIO_Init(GPIOB,&GPIO_InitStrcture);     
	GPIO_InitStrcture.GPIO_Pin=GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_InitStrcture);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStrcture;   //配置TIM2
	TIM_TimeBaseStructInit(&TIM_TimeBaseStrcture);
	TIM_TimeBaseStrcture.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseStrcture.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseStrcture.TIM_Period=65535;
	TIM_TimeBaseStrcture.TIM_Prescaler=0;  
	TIM_TimeBaseStrcture.TIM_RepetitionCounter=0;
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStrcture);
	
//	TIM_ICInitTypeDef TIM_ICInitStrcture;
//	TIM_ICStructInit(&TIM_ICInitStrcture);
//	TIM_ICInitStrcture.TIM_Channel=TIM_Channel_1 | TIM_Channel_2;
//	TIM_ICInitStrcture.TIM_ICFilter=0x00;  //加入硬件滤波，如果数据跳动太严重则需要启动
//	TIM_ICInit(TIM2,&TIM_ICInitStrcture);
	
	TIM_EncoderInterfaceConfig(TIM2,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);
	
	TIM_SetCounter(TIM2,0);
	TIM_Cmd(TIM2,ENABLE);
	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);     //配置TIM4
	
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStrcture);
	
//	TIM_ICInitTypeDef TIM_ICInitStrcture;
//	TIM_ICStructInit(&TIM_ICInitStrcture);
//	TIM_ICInitStrcture.TIM_Channel=TIM_Channel_1 | TIM_Channel_2;
//	TIM_ICInitStrcture.TIM_ICFilter=0x00;  //加入硬件滤波，如果数据跳动太严重则需要启动
//	TIM_ICInit(TIM2,&TIM_ICInitStrcture);
	
	TIM_EncoderInterfaceConfig(TIM4,TIM_EncoderMode_TI12,TIM_ICPolarity_Falling,TIM_ICPolarity_Falling);  //
	
	TIM_SetCounter(TIM4,0);
	TIM_Cmd(TIM4,ENABLE);
	//用(int16_t)TIM_GetCounter(TIMx)来获取读数，强转的原因是这个函数是uint16_t的函数
}

int16_t GetLeftEncoder(void){  //获取编码器的数据并处理成实际轮速
	return (int16_t)TIM_GetCounter(TIM2) / 44.0 / 0.05 / 9.27666;
}

int16_t GetRightEncoder(void){
	return -(int16_t)TIM_GetCounter(TIM4)  / 44.0 / 0.05 / 9.27666;
}

float GetLeftEncoder_Speed(void){  //获取编码器的增量数据并处理成实际轮速，对比上方函数此函数调用后会清空计数值,输出的是增量而不是计数值
	float temp=0;
	temp=(int16_t)TIM_GetCounter(TIM2) / 44.0 / 0.005 / 9.27666;
	TIM_SetCounter(TIM2 ,0);
	return temp;
}

float GetRightEncoder_Speed(void){
	float temp1=0;
	temp1=-(int16_t)TIM_GetCounter(TIM4) / 44.0 / 0.005 / 9.27666;
	TIM_SetCounter(TIM4 ,0);
	return temp1;
}

