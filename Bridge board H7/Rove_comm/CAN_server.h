#ifndef CAN_server_H
#define CAN_server_H

#include <stdint.h>
#include <stddef.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"

typedef union{
	uint32_t raw;
	struct{
		uint32_t src : 4;
		uint32_t dest : 4;
		uint32_t data_ID : 10;
		uint32_t type : 4;
		uint32_t length : 6;
		uint32_t reserved : 4;
	} header;
} CAN_ID_h;

void RC_CAN_init(FDCAN_HandleTypeDef *hfdcan1, uint8_t ID);
void RC_CAN_send(uint8_t dest, uint16_t data_ID, uint8_t length, uint8_t data_Type);
void RC_CAN_Receive(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

#endif
