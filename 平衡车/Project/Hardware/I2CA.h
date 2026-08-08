#ifndef __I2CA_H
#define __I2CA_H

void I2CA_Init(void);
void Set_SCL(uint8_t BitValue);
void Set_SDA(uint8_t BitValue);
uint8_t Get_SDA(void);
void I2CA_Start(void);
void I2CA_Stop(void);
void I2CA_Send(uint8_t Byte);
uint8_t I2CA_Receive(void);
void I2CA_SendAck(void);
void I2CA_SendNoAck(void);
uint8_t I2CA_ReceiveAck(void);


#endif
