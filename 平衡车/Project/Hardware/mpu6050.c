#include "stm32f10x.h"                  // Device header
#include "Delay.h"						//延时

#include "I2CA.h"

#include "math.h"

uint8_t MPU6050_WhoAmI(void){
	uint8_t test1;
	I2CA_Init();
	Delay_us(100);
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x75);
	I2CA_ReceiveAck();
	I2CA_Start();
	I2CA_Send(0xD1);
	I2CA_ReceiveAck();
	test1=I2CA_Receive();
	I2CA_SendNoAck();
	I2CA_Stop();
	return test1;

}

void MPU6050_Init(void){
	I2CA_Init();
	Delay_us(100);
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x19); 
	I2CA_ReceiveAck();
	I2CA_Send(0x00);  //对寄存器19 采样率分频寄存器赋初始值0，不分频
	I2CA_ReceiveAck();
	I2CA_Stop();
	
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x1A); 
//	I2CA_ReceiveAck();
//	I2CA_Send(0x10);  //对寄存器1A 配置寄存器赋值0000 0000 DLPF设置为0
//	I2CA_ReceiveAck();
//	I2CA_Stop();
	
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x1B); 
	I2CA_ReceiveAck();
	I2CA_Send(0x18);  //对寄存器1B 陀螺仪配置寄存器赋值0001 000 FS_SEL角度量程选择为正负2000度
	I2CA_ReceiveAck();
	I2CA_Stop();
	
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x1C); 
	I2CA_ReceiveAck();
	I2CA_Send(0x18);  //对寄存器1C 加速度计配置寄存器赋值0001 1000 ACCEL_FS_SEL加速度量程选择为正负16G
	I2CA_ReceiveAck();
	I2CA_Stop();
	
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x6B); 
	I2CA_ReceiveAck();
	I2CA_Send(0x01);  //对寄存器6B 电源管理1寄存器赋值0000 0001，关闭睡眠模式并把时钟源设置为陀螺仪X轴 
	I2CA_ReceiveAck();
	I2CA_Stop();
	
}

int16_t MPU6050_GetGyroX(void){
	uint8_t DataH,DataL;
	int16_t GyroX;
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x43);   //发送GyroX输出高位寄存器的地址  0x43
	I2CA_ReceiveAck();
	I2CA_Start();
	I2CA_Send(0xD1);
	I2CA_ReceiveAck();
	DataH=I2CA_Receive();  //接收高位数据
	I2CA_SendAck();
	DataL=I2CA_Receive();  //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
	I2CA_SendNoAck();
	I2CA_Stop();
	GyroX=((DataH << 8) | DataL);  //将高位数据向高位移动8位，然后将低位数据或进去
	return GyroX;

}

//int16_t MPU6050_GetAccelX(void){
//	uint8_t DataH,DataL;
//	int16_t AccelX;
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x3B);    //发送AccelX输出高位寄存器的地址  0x3B
//	I2CA_ReceiveAck();
//	I2CA_Start();
//	I2CA_Send(0xD1);
//	I2CA_ReceiveAck();
//	DataH=I2CA_Receive();   //接收高位数据
//	I2CA_SendAck();
//	DataL=I2CA_Receive();   //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
//	I2CA_SendNoAck();
//	I2CA_Stop();
//	AccelX=((DataH << 8) | DataL);   //将高位数据向高位移动8位，然后将低位数据或进去
//	return AccelX;

//}

//int16_t MPU6050_GetGyroY(void){
//	uint8_t DataH,DataL;
//	int16_t GyroY;
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x45);   //发送GyroY输出高位寄存器的地址  0x45
//	I2CA_ReceiveAck();
//	I2CA_Start();
//	I2CA_Send(0xD1);
//	I2CA_ReceiveAck();
//	DataH=I2CA_Receive();  //接收高位数据
//	I2CA_SendAck();
//	DataL=I2CA_Receive();  //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
//	I2CA_SendNoAck();
//	I2CA_Stop();
//	GyroY=((DataH << 8) | DataL);  //将高位数据向高位移动8位，然后将低位数据或进去
//	return GyroY;

//}

//int16_t MPU6050_GetAccelY(void){
//	uint8_t DataH,DataL;
//	int16_t AccelY;
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x3D);    //发送AccelY输出高位寄存器的地址  0x3D
//	I2CA_ReceiveAck();
//	I2CA_Start();
//	I2CA_Send(0xD1);
//	I2CA_ReceiveAck();
//	DataH=I2CA_Receive();   //接收高位数据
//	I2CA_SendAck();
//	DataL=I2CA_Receive();   //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
//	I2CA_SendNoAck();
//	I2CA_Stop();
//	AccelY=((DataH << 8) | DataL);   //将高位数据向高位移动8位，然后将低位数据或进去
//	return AccelY;

//}

//int16_t MPU6050_GetGyroZ(void){
//	uint8_t DataH,DataL;
//	int16_t GyroZ;
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x47);   //发送GyroZ输出高位寄存器的地址  0x47
//	I2CA_ReceiveAck();
//	I2CA_Start();
//	I2CA_Send(0xD1);
//	I2CA_ReceiveAck();
//	DataH=I2CA_Receive();  //接收高位数据
//	I2CA_SendAck();
//	DataL=I2CA_Receive();  //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
//	I2CA_SendNoAck();
//	I2CA_Stop();
//	GyroZ=((DataH << 8) | DataL);  //将高位数据向高位移动8位，然后将低位数据或进去
//	return GyroZ;

//}

//int16_t MPU6050_GetAccelZ(void){
//	uint8_t DataH,DataL;
//	int16_t AccelZ;
//	I2CA_Start();
//	I2CA_Send(0xD0);
//	I2CA_ReceiveAck();
//	I2CA_Send(0x3F);    //发送AccelZ输出高位寄存器的地址  0x3F
//	I2CA_ReceiveAck();
//	I2CA_Start();
//	I2CA_Send(0xD1);
//	I2CA_ReceiveAck();
//	DataH=I2CA_Receive();   //接收高位数据
//	I2CA_SendAck();
//	DataL=I2CA_Receive();   //接收低位数据，第一个数据发送后接着读取会按照地址顺序依次读取
//	I2CA_SendNoAck();
//	I2CA_Stop();
//	AccelZ=((DataH << 8) | DataL);   //将高位数据向高位移动8位，然后将低位数据或进去
//	return AccelZ;

//}

void MPU6050_GetAccelGyroReg(uint8_t *Data_Array){
	uint8_t i;
	I2CA_Start();
	I2CA_Send(0xD0);
	I2CA_ReceiveAck();
	I2CA_Send(0x3B);    //发送AccelX输出高位寄存器的地址  0x3B，开始按顺序读取寄存器数据并存放在数组中
	I2CA_ReceiveAck();
	I2CA_Start();
	I2CA_Send(0xD1);
	I2CA_ReceiveAck();
	for(i=0;i<14;i++){
		Data_Array[i]=I2CA_Receive();
		if(i<13){
			I2CA_SendAck();   //未读取到最后一个字节之前一直发送应答继续读取
		}
			else {
				I2CA_SendNoAck();
			}
	}
	I2CA_Stop();
}

void MPU6050_GetData(int16_t *AccelX,int16_t *AccelY,int16_t *AccelZ,int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ){
	uint8_t Data[14];
	MPU6050_GetAccelGyroReg(Data);
	
	*AccelX = (Data[0] << 8) | Data[1];		//Data[0]和Data[1]为加速度计X轴数据
	*AccelY = (Data[2] << 8) | Data[3];		//Data[2]和Data[3]为加速度计Y轴数据
	*AccelZ = (Data[4] << 8) | Data[5];		//Data[4]和Data[5]为加速度计Z轴数据
	
	*GyroX = (Data[8] << 8) | Data[9];		//Data[8]和Data[9]为陀螺仪X轴数据
	*GyroY = (Data[10] << 8) | Data[11];	//Data[10]和Data[11]为陀螺仪Y轴数据
	*GyroZ = (Data[12] << 8) | Data[13];	//Data[12]和Data[13]为陀螺仪Z轴数据
}



float MPU6050_GyroXCalibrate(void){
		uint8_t Gyro_InitTimer;
		float GyroX_Shift=0;
	   //第一次启动时采集200次GryoX的数据用于消除误差
		for(Gyro_InitTimer=0;Gyro_InitTimer<200;Gyro_InitTimer++){
		GyroX_Shift+=MPU6050_GetGyroX()/16.4;  
		Delay_ms(2);
		}
		GyroX_Shift= GyroX_Shift/200;
		return GyroX_Shift;
}

float MPU6050_GetFilteredAngle(float GyroX_Shift,int16_t GyroX,int16_t AccelX,int16_t AccelY,int16_t AccelZ){
	
	static float Pitch_angle=0;
	float alpha=0.98; //滤波权重系数
	
	float Gyro_X = GyroX / 16.4 - GyroX_Shift;  //X轴的角速度
	
	float Accel_Pitch = atan2(AccelY,sqrt( (AccelX*AccelX) + (AccelZ*AccelZ) )) *180 / 3.14159;   //通过加速度计计算角度
	
	float Gyro_Pitch = Pitch_angle + Gyro_X * 0.005; //通过陀螺仪计算角度，计算周期5ms
	
	Pitch_angle = alpha * Gyro_Pitch + (1-alpha) * Accel_Pitch;
	
	return Pitch_angle;
	
}

float MPU6050_GetProcessedGyroX(float GyroX_Shift,int16_t GyroX){
	float Gyro_X = GyroX / 16.4 - GyroX_Shift;
	return Gyro_X;
}

