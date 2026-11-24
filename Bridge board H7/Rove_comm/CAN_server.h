#ifndef CAN_server_H
#define CAN_server_H

#include <stdint.h>
#include <stddef.h>

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
#endif
