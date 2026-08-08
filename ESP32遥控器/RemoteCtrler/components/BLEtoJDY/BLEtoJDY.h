#ifndef __BLETOJDY_H_
#define __BLETOJDY_H_

int BLE_Send_Data(const uint8_t *data, uint16_t len);
void BLE_Init(void);

void BLE_Send_Joystick_Packet(int LeftHorizon,int LeftVertical,int RightHorizon,int RightVertical);
void BLE_Send_Key_Packet(int Key1,int Key2,int Key3,int Key4);

#endif
