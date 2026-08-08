#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

char BlueSerial_RxPacket[100];		
uint8_t BlueSerial_RxFlag;			//定义蓝牙串口接收的标志位变量

void Bluetooth_Init(void){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;   //PB11设置为上拉输入
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;		//PB10设置为复用推挽输出
		GPIO_Init(GPIOB,&GPIO_InitStructure);
	
		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate=115200;  //波特率设置115200
		USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;  //设置无硬件流控制
		USART_InitStructure.USART_Mode=USART_Mode_Tx | USART_Mode_Rx;  //设置 Tx和Rx模式同时开启
		USART_InitStructure.USART_Parity=USART_Parity_No;  //设置 无效验
		USART_InitStructure.USART_StopBits=USART_StopBits_1;  //设置 停止位1
		USART_InitStructure.USART_WordLength=USART_WordLength_8b;		//字长设置8bit
		USART_Init(USART3,&USART_InitStructure);
	
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //使能USART3中断
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //设置中断优先组2
		
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=2;
		NVIC_Init(&NVIC_InitStructure);
		
		USART_Cmd(USART3,ENABLE);
	
}

void Bluetooth_SendByte(uint8_t Byte){
	USART_SendData(USART3,Byte);   
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);  //等待发送完成
}

void Bluetooth_SendString(char *String){
	uint8_t i;
	for(i=0; String[i] != '\0' ;i++){
		Bluetooth_SendByte(String[i]);
	}
}
uint16_t Bluetooth_ReceiveData(void){
	return USART_ReceiveData(USART3);
}

int fputc(int ch, FILE *f)
{
	Bluetooth_SendByte(ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Serial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	Bluetooth_SendString(String);		//串口发送字符数组（字符串）
}




void USART3_IRQHandler(void){
	
		static uint8_t RxState = 0;		//定义表示当前状态机状态的静态变量
	static uint8_t pRxPacket = 0;	//定义表示当前接收数据位置的静态变量
		
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)	//判断是否是USART2的接收事件触发的中断
	{
		uint8_t RxData = USART_ReceiveData(USART3);			//读取数据寄存器，存放在接收的数据变量
		
		/*使用状态机的思路，依次处理数据包的不同部分*/
		
		/*当前状态为0，接收数据包包头*/
		if (RxState == 0)
		{
			if (RxData == '[' && BlueSerial_RxFlag == 0)		//如果数据确实是包头，并且上一个数据包已处理完毕
			{
				RxState = 1;			//置下一个状态
				pRxPacket = 0;			//数据包的位置归零
			}
		}
		/*当前状态为1，接收数据包数据，同时判断是否接收到了包尾*/
		else if (RxState == 1)
		{
			if (RxData == ']')			//如果收到包尾
			{
				RxState = 0;			//状态归0
				BlueSerial_RxPacket[pRxPacket] = '\0';			//将收到的字符数据包添加一个字符串结束标志
				BlueSerial_RxFlag = 1;	//接收数据包标志位置1，成功接收一个数据包
			}
			else						//接收到了正常的数据
			{
				BlueSerial_RxPacket[pRxPacket] = RxData;		//将数据存入数据包数组的指定位置
				pRxPacket ++;			//数据包的位置自增
			}
		}

		}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	
