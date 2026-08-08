#include "stm32f10x.h"                  // Device header
#include "Delay.h"						//延时  I2C软件模拟刚需Delay，放弃使用Systick
#include "OLED.h"						//OLED驱动
#include "PWMA.h"
#include "I2CA.h"
#include "mpu6050.h"
#include "Encoder.h"
#include "JDY31.h"
#include "PID.h"

#include "String.h"
#include "stdlib.h"

uint8_t Boot=0,RemoteCtrl_Standup=0,PID_State=0,StandUp_State=0,RemoteCtrl_WatchDog_Move=0,StandUp_Timer=0;
uint8_t SafetyProtection=1;
uint8_t TIM3_TickCounter=0;
uint16_t Bluetooth_RxData[3];  
uint8_t RxDataCounter=0;

int16_t AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ;

float LeftSpeed,RightSpeed;

float Actual_Angle,Error0_Angle,Error1_Angle;
float Actual_Steering,Error0_Steering,Error1_Steering;
float Actual_Speed,Error0_Speed,ErrorInt_Speed;
float Target_Angle,Out_Angle,Out_Speed,Out_Steering,PWM_Target_Left,PWM_Target_Right,PWM_Dif,Speed_Dif;
float FilteredAngle,GyroX_Shift,GyroX_Speed;

float Kp_Speed=0.80,Ki_Speed=0.02;

float Kp_Angle=12.75*0.7,Kd_Angle=0.44*0.8;

float Kp_Steering=1.27,Kd_Steering=0.20;

float Target_Speed=0,Target_Steering=0;

int8_t JoyStick_LH,JoyStick_LV,JoyStick_RH,JoyStick_RV;

int main(void){
	OLED_Init();
	PWMA_Init();
	Delay_ms(150);
	MPU6050_Init();
	Delay_ms(250);
	MPU6050_Init();
	Encoder_Init();
	Bluetooth_Init();
	PID_Init();
	
while(1){
//		OLED_ShowNum(2,1,Kp,5);
//		OLED_ShowNum(3,1,Ki,5);
//		OLED_ShowNum(4,1,Kd,5);
	OLED_ShowString(1,1,"Speed");
	OLED_ShowString(2,1,"PWM");
	OLED_ShowString(3,1,"Angle");
	OLED_ShowString(4,1,"Gyro");
	
	
	OLED_ShowSignedNum(1,6,PWM_Target_Left,4);
	OLED_ShowSignedNum(2,6,PWM_Target_Right,4);
	OLED_ShowSignedNum(3,6,FilteredAngle,3);
	OLED_ShowSignedNum(4,6,GyroX_Speed,3);
	
	OLED_ShowSignedNum(1,12,JoyStick_LV,3);
	OLED_ShowSignedNum(2,12,JoyStick_RH,3);
	OLED_ShowSignedNum(3,12,RemoteCtrl_Standup,3);
	OLED_ShowSignedNum(4,12,StandUp_State,3);
	
	
	if (BlueSerial_RxFlag == 1){
		
		char *Tag = strtok(BlueSerial_RxPacket,",");  //提取第一个数据作为Tag
		
		if (strcmp(Tag, "key") == 0 ){  //Tag如果为key，则为收到按键数据包
			
				char *Action1 = strtok(NULL, ",");				//提取数据2，定义为按键动作1
				char *Action2 = strtok(NULL, ",");			//提取数据3，定义为按键动作2
			
				if(Action1 != NULL && strncmp(Action1, "1", 1) == 0)RemoteCtrl_Standup=1;
				else {
					RemoteCtrl_Standup=0;
					PID_State=0;
					StandUp_State=0;
					StandUp_Timer=0;
					PWMA_SetLeftPWM(0);
					PWMA_SetRightPWM(0);
					ErrorInt_Speed=0; //关闭自平衡后清除PID缓存，防止出现意料之外的指令
				}
			}
		else if (strcmp(Tag, "joy") == 0 ){
			
				RemoteCtrl_WatchDog_Move=0;
			
				JoyStick_LH = atoi(strtok(NULL, ","));		//提取数据2，定义为摇杆值JoyStick_LH
				JoyStick_LV = atoi(strtok(NULL, ","));		//提取数据3，定义为摇杆值JoyStick_LV
				JoyStick_RH = atoi(strtok(NULL, ","));		//提取数据4，定义为摇杆值JoyStick_RH
				JoyStick_RV = atoi(strtok(NULL, ","));		//提取数据5，定义为摇杆值JoyStick_RV
			
				Target_Speed = JoyStick_LV / 20;  //将左摇杆垂直方向的数据交给速度环
				Target_Steering = JoyStick_RH / 4;   //将右摇杆水平方向的数据交给转向环
				
		}
		
		BlueSerial_RxFlag = 0;  //处理完成后标志位置0允许处理下一个数据包
	}
	
	
//	Serial_Printf("%.2f,%.2f,%.2f,%.2f\r\n", Actual_Angle,Out_Angle,Kp_Angle * Error0_Angle,Kd_Angle * GyroX_Speed);   //串口输出参数用于调参
	
}	
	
}

void TIM3_IRQHandler(void){  //PID等都放在这里  等效5ms进一次中断
	if (TIM_GetITStatus(TIM3,TIM_IT_Update)==SET){
		if(Boot==0){
			GyroX_Shift=MPU6050_GyroXCalibrate();
			Boot++;
		}
		
		
		
				
		if (TIM3_TickCounter >= 5){
			RemoteCtrl_WatchDog_Move++;
			if(RemoteCtrl_WatchDog_Move>=100){  //看门狗  等效50ms  如果事件范围内没有收到动作数据包强制将动作数据全部归零
				JoyStick_LH =0;
				JoyStick_LV =0;
				JoyStick_RH =0;
				JoyStick_RV =0;
				Target_Speed =0;
				Target_Steering =0;
				Out_Steering=0;
				PWM_Dif=0;
			}
			
			
			TIM3_TickCounter=0; 
			
			MPU6050_GetData(&AccelX,&AccelY,&AccelZ,&GyroX,&GyroY,&GyroZ);
			FilteredAngle=-MPU6050_GetFilteredAngle(GyroX_Shift,GyroX,AccelX,AccelY,AccelZ) + 0;  //获取滤波后的姿态角度和重心偏移量纠正
			GyroX_Speed=-MPU6050_GetProcessedGyroX(GyroX_Shift,GyroX);
			LeftSpeed=GetLeftEncoder_Speed();   //获取轮速
			RightSpeed=GetRightEncoder_Speed();
			Speed_Dif = LeftSpeed - RightSpeed;
			
			/*自启动部分代码*/
		
			if(StandUp_State==0 && RemoteCtrl_Standup==1){
					if(FilteredAngle>=25){
						StandUp_State=1;  //检测到小车前倾
						StandUp_Timer=0;
					}
					else if(FilteredAngle<=-25){
						StandUp_State=2;  //检测到小车后倾
						StandUp_Timer=0;
					}
				}
					if(StandUp_State==1) {  //证实如果是前倾就给前进动力，否则进入下一阶段
						if(FilteredAngle>=25){
							PWMA_SetLeftPWM(70);
							PWMA_SetRightPWM(70);
							
						}
						else if(FilteredAngle<25 && FilteredAngle > -25){
							PWMA_SetLeftPWM(0);
							PWMA_SetRightPWM(0);
							StandUp_State=3;
						}
					}
					if(StandUp_State==2) {  //证实如果是后倾就给后退动力，否则进入下一阶段
						if(FilteredAngle<=-25){
							PWMA_SetLeftPWM(-70);
							PWMA_SetRightPWM(-70);
						}
						else if(FilteredAngle<25 && FilteredAngle > -25){
							PWMA_SetLeftPWM(0);
							PWMA_SetRightPWM(0);
							StandUp_State=3;
						}
					}
					
					if(StandUp_State == 3 && FilteredAngle<25 && FilteredAngle > -25){  //如果角度值已经在范围内则启动PID
						StandUp_State=4;
						PWMA_SetLeftPWM(0);
						PWMA_SetRightPWM(0);
						ErrorInt_Speed=0;
						Error0_Speed=0;
						Target_Speed =0;
						Actual_Speed=0;
						Out_Speed=0;
						Target_Steering =0;
						PID_State=1;
					
				}
		
			
			/*自启动部分代码结束*/
			
			if(PID_State==1){
			
			if (FilteredAngle>35 || FilteredAngle<-35){   //安全保护，倾斜角大于35切断动力输出
				PWMA_SetLeftPWM(0);
				PWMA_SetRightPWM(0);
				ErrorInt_Speed=0; //清空速度环Ki防止PID恢复后猛冲出去
				SafetyProtection=0;
			}else SafetyProtection=1;
			
			if (SafetyProtection ==1){
				
			
				//直立环 PD
				
				
				Actual_Angle = FilteredAngle;  //获取实际姿态
				
				Error1_Angle=Error0_Angle;
				Error0_Angle= Target_Angle - Actual_Angle;
			
				
				Out_Angle = Kp_Angle * Error0_Angle - Kd_Angle * GyroX_Speed;
				
				PWM_Target_Left=-Out_Angle + PWM_Dif / 2;
				PWM_Target_Right=-Out_Angle - PWM_Dif / 2;
				
				if (PWM_Target_Left>100)PWM_Target_Left=100;
				if (PWM_Target_Left<-100)PWM_Target_Left=-100;   //左轮输出限幅
				
				if (PWM_Target_Right>100)PWM_Target_Right=100;
				if (PWM_Target_Right<-100)PWM_Target_Right=-100;   //右轮输出限幅
				
				PWMA_SetLeftPWM(PWM_Target_Left);
				PWMA_SetRightPWM(PWM_Target_Right); //将计算出的PWM输出给电机
				
				
					//速度环 PI
				
				Actual_Speed = (LeftSpeed + RightSpeed) / 2.0;  //获取当前编码器位置
				
				Error0_Speed= Target_Speed - Actual_Speed;
				
				ErrorInt_Speed+= Error0_Speed;
				
				Out_Speed = Kp_Speed * Error0_Speed + Ki_Speed * ErrorInt_Speed;
				
				if (Out_Speed>15)Out_Speed=15;
				if (Out_Speed<-15)Out_Speed=-15;   //输出限幅
				
				Target_Angle=Out_Speed;
				
				//转向环 PD
				
				
				Actual_Steering = Speed_Dif;  //获取实际姿态
				
				Error1_Steering=Error0_Steering;
				Error0_Steering= Target_Steering - Actual_Steering;
			
				
				Out_Steering = Kp_Steering * Error0_Steering - Kd_Steering * (Error1_Steering - Error0_Steering);
				
				if (Out_Steering>30)Out_Steering=30;
				if (Out_Steering<-30)Out_Steering=-30;   //输出限幅
				
				PWM_Dif=Out_Steering;
				
				}
			}
			
		}
		else TIM3_TickCounter++;
	}
	
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
}

