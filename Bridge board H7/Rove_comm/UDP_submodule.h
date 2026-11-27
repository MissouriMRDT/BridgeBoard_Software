
#ifndef UDP_submodule_H
#define UDP_submodule_H

#include <stdint.h>
#include <stddef.h>
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/def.h"
#define RC_maxPayload 64
#define RC_Version 0x01
#define RC_Port 11000
#define maxSubscriber 14
#define maxSubmodules 10


/*typedef struct {
	uint8_t version;
	uint16_t Data_ID;
	uint16_t Count;
	uint8_t Data_type;

} RC_header_t;*/



typedef struct{
	uint8_t CAN_ID;
	ip_addr_t IPaddr;
	ip_addr_t subscribers[maxSubscriber];
	struct udp_pcb *upcb;
} RC_submodule_h;

//extern ip_addr_t substribers[14];
//
void udpSubmodule_init(uint8_t CAN_ID, uint8_t addr1, uint8_t addr2, uint8_t addr3, uint8_t addr4);
//void RC_Send_Blank(struct udp_pcb *pcb, const ip_addr_t *send_IP, uint16_t data_id);
void submodule_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
//void add_subscriber(const ip_addr_t *addr);
//void remove_subscriber(const ip_addr_t *addr);
//void Send_subscriber(uint16_t data_id, uint16_t data_length, uint8_t data_size,uint8_t data_type, const void *data);
//void RC_functionCallack(RC_header_t hdr, RC_payload_t *payload);
//




#endif
