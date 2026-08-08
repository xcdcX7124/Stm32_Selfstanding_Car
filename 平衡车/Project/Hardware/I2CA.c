#include "stm32f10x.h"                  // Device header
#include "Delay.h"


void I2CA_Init(void){

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//PB3和PB4默认被Jlink占用，需要解除占用以后才能使用这两个端口
	
	GPIO_InitTypeDef GPIOStructure;
	GPIOStructure.GPIO_Mode=GPIO_Mode_Out_OD; //配置PB3和PB4为复用开漏输出
	GPIOStructure.GPIO_Pin=GPIO_Pin_3;
	GPIOStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIOStructure);
	GPIOStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_Init(GPIOB,&GPIOStructure);
		
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);  //配置I2C1
	//I2C_InitTypeDef I2C_InitStructure;
	//I2C_StructInit(&I2C_InitStructure);
	//I2C_InitStructure.I2C_Ack=I2C_Ack_Enable; //应答使能
	//I2C_InitStructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit; //7位地址
	//I2C_InitStructure.I2C_ClockSpeed=100000;  //速度设置为100khz
	//I2C_InitStructure.I2C_DutyCycle=I2C_DutyCycle_2;
	//I2C_InitStructure.I2C_Mode=I2C_Mode_I2C;
	//I2C_InitStructure.I2C_OwnAddress1=0x00; //I2C的自身地址，因为是固定主机所以给了个0x00占位
	
	//I2C_Init(I2C1,&I2C_InitStructure);
	//I2C_Cmd(I2C1,ENABLE);
	//他的板子设计的是PB3和PB4是给I2C用，但PB6和7才是给I2C用的针脚而且他拿去给电机用了，没得用硬件I2C只能用软件模拟了
	
	GPIO_SetBits(GPIOB,GPIO_Pin_3);
	GPIO_SetBits(GPIOB,GPIO_Pin_4);  //配置PB3和PB4为默认的高电平
	
}

void Set_SCL(uint8_t BitValue){
	GPIO_WriteBit(GPIOB,GPIO_Pin_4,(BitAction)BitValue);  //封装PB4 SCL针脚的使能失能函数方便调用
		Delay_us(10);
}


void Set_SDA(uint8_t BitValue){
	GPIO_WriteBit(GPIOB,GPIO_Pin_3,(BitAction)BitValue);  //封装PB3 SDA针脚的使能失能函数方便调用
	Delay_us(10);
}

uint8_t Get_SDA(){
	uint8_t BitValue;
	BitValue=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3);  //获取PB3 SDA针脚的电平方便调用
	Delay_us(10);
	return BitValue;
	
}

void I2CA_Start(void){  //起始条件：SCL高电平时SDA从高电平变成低电平
	
	Set_SDA(1);
	Set_SCL(1);  //确保两个引脚均为高电平
	Set_SDA(0);
	Set_SCL(0);  //依次拉低电平
		
}

void I2CA_Stop(void){  //终止条件：SCL高电平时SDA从低电平变成高电平
	
	Set_SDA(0);  //先拉低SDA防止SDA还在高电平
	Set_SCL(1);
	Set_SDA(1);
		
}

void I2CA_Send(uint8_t Byte){
	uint8_t i;
	for (i=0;i<8;i++){
		//Set_SCL(0);   //拉低SCL准备把数据放在SDA上
		Set_SDA( Byte & (0x80>>i));  //通过掩码的方式从高到低位将数据移到SDA上，共循环8次
		Set_SCL(1);    //释放SCL让从机读取数据
		Set_SCL(0);   //拉低SCL准备再次输出数据或在输出结束时拉低SCL结束输出
	}
}

uint8_t I2CA_Receive(void){
	uint8_t i;
	uint8_t Byte=0x00;  //初始化缓存字节
		Set_SDA(1);  //释放SDA
	for (i=0;i<8;i++){
		Set_SCL(1);  //释放SCL  读取从机发送的数据
		if(Get_SDA()==1){
			Byte |= (0x80 >> i);  //如果从机在此位输出了1则把数据存到相应位置的缓存中
		}
		Set_SCL(0);  //拉低SCL，让从机输出下一位数据
	}
	return Byte;
}

void I2CA_SendAck(void){
	Set_SDA(0); //拉低SDA表示应答
	Set_SCL(1); //释放SCL让从机读取
	Set_SCL(0);  //拉低SCL准备下一个模块
}

void I2CA_SendNoAck(void){
	Set_SDA(1); //释放SDA表示非应答
	Set_SCL(1); //释放SCL让从机读取
	Set_SCL(0);  //拉低SCL准备下一个模块
}

uint8_t I2CA_ReceiveAck(void){
	Set_SDA(1); //释放SDA避免干扰从机应答
	Set_SCL(1); //释放SCL并读取应答数据
	if (Get_SDA()==1){
		Set_SCL(0);  //拉低SCL准备下一个模块
		return 1;
	}
	else {
		Set_SCL(0);  //拉低SCL准备下一个模块
		return 0;
	}
}

