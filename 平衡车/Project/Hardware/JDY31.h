#ifndef __JDY31_H
#define __JDY31_H

#include <stdio.h>

void Bluetooth_Init(void);
void Bluetooth_SendByte(uint8_t Byte);
void Bluetooth_SendString(char *String);
uint16_t Bluetooth_ReceiveData(void);

void Serial_Printf(char *format, ...);

extern char BlueSerial_RxPacket[100];		
extern uint8_t BlueSerial_RxFlag;	

#endif
