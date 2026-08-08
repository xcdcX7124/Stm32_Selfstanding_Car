#ifndef __MPU6050_H
#define __MPU6050_H

uint8_t MPU6050_WhoAmI(void);
void MPU6050_Init(void);
//int16_t MPU6050_GetGyroX(void);
//int16_t MPU6050_GetAccelX(void);
//int16_t MPU6050_GetGyroY(void);
//int16_t MPU6050_GetAccelY(void);
//int16_t MPU6050_GetGyroZ(void);
//int16_t MPU6050_GetAccelZ(void);
void MPU6050_GetData(int16_t *AccelX,int16_t *AccelY,int16_t *AccelZ,int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ);

float MPU6050_GyroXCalibrate(void);

float MPU6050_GetFilteredAngle(float GyroX_Shift,int16_t GyroX,int16_t AccelX,int16_t AccelY,int16_t AccelZ);;
float MPU6050_GetProcessedGyroX(float GyroX_Shift,int16_t GyroX);
#endif
