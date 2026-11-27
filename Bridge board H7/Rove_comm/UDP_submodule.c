//#include "lwip/udp.h"
//#include "lwip/pbuf.h"
//#include "stdio.h"
//#include "lwip/api.h"
//#include "lwip/sys.h"
//#include "lwip/ip_addr.h"
//#include "lwip/init.h"
//#include "lwip/def.h"
//#include <string.h>
//#include <stdint.h>
//#include "UDP_server.h"
//#include "lwip/udp.h"
//#include "main.h"

#include "UDP_submodule.h"
#include "UDP_server.h"
#include "CAN_server.h"
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/def.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include <string.h>

extern struct netif gnetif;


static RC_submodule_h UDP_Submodules[maxSubmodules];
static uint8_t UDP_Submodule_Lookup[maxSubmodules] ={0};
static struct netif UDP_submodule_netifs[maxSubmodules];

static err_t virtual_netif_init(struct netif *netif){
	netif->name[0] ='v';
	netif->name[1]= '0';
	netif->output = etharp_output;
	netif->linkoutput = gnetif.linkoutput;
	netif->mtu = gnetif.mtu;
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
	memcpy(netif->hwaddr, gnetif.hwaddr, ETHARP_HWADDR_LEN);
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	return ERR_OK;
	}

static err_t multi_ip_tcpip_input(struct pbuf *p, struct netif *inp){
	struct netif *target_netif = inp;

	    if (p != NULL && p->len >= 42) {
	        uint8_t *eth_hdr = (uint8_t *)p->payload;
	        uint16_t eth_type = (eth_hdr[12] << 8) | eth_hdr[13];
	        ip4_addr_t target_ip;

	        if (eth_type == 0x0806) {
	            IP4_ADDR(&target_ip, eth_hdr[38], eth_hdr[39], eth_hdr[40], eth_hdr[41]);
	        } else if (eth_type == 0x0800) {
	            IP4_ADDR(&target_ip, eth_hdr[30], eth_hdr[31], eth_hdr[32], eth_hdr[33]);
	        }

	        if (eth_type == 0x0806 || eth_type == 0x0800) {
	            for (struct netif *n = netif_list; n != NULL; n = n->next) {
	                if (ip4_addr_cmp(netif_ip4_addr(n), &target_ip)) {
	                    target_netif = n;
	                    break;
	                }
	            }
	        }
	    }


	    return tcpip_input(p, target_netif);

}

void udpSubmodule_init(uint8_t CAN_ID, uint8_t addr1, uint8_t addr2, uint8_t addr3, uint8_t addr4){
	LOCK_TCPIP_CORE();
	gnetif.input = multi_ip_tcpip_input;


	uint8_t slot;
	for(uint8_t i =0; i<maxSubmodules ; i++){
		if(UDP_Submodule_Lookup[i] ==0){
			slot =i;
			break;}
		}

	ip4_addr_t ipaddr, netmask, gw;
	IP4_ADDR(&ipaddr, addr1, addr2, addr3, addr4);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 2, 1);

	struct netif *v_netif = &UDP_submodule_netifs[slot];
	memset(v_netif, 0, sizeof(struct netif));

	if(netif_add(v_netif, &ipaddr, &netmask, &gw, NULL, virtual_netif_init, tcpip_input) ==NULL){
		UNLOCK_TCPIP_CORE();
		return;
	}
	netif_set_up(v_netif);
	netif_set_link_up(v_netif);
	etharp_gratuitous(v_netif);

	struct udp_pcb *upcb = udp_new();

	err_t err = udp_bind(upcb, &ipaddr, 11000);
	if(err == ERR_OK){
		UDP_Submodule_Lookup[slot] = CAN_ID;
		UDP_Submodules[slot].CAN_ID = CAN_ID;
		UDP_Submodules[slot].IPaddr = ipaddr;
		UDP_Submodules[slot].upcb = upcb;

		udp_recv(upcb, submodule_receive_callback, &UDP_Submodules[slot]);
	}
	else{
		udp_remove(upcb);
		netif_remove(v_netif);
	}
	UNLOCK_TCPIP_CORE();
}



void submodule_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port){


	RC_header_t hdr;
	RC_header_t hdr_raw;
	static RC_payload_t payload;
	RC_submodule_h *submodule = (RC_submodule_h *)arg;




	pbuf_copy_partial(p, &hdr_raw, sizeof(RC_header_t),0);

	hdr.version =hdr_raw.version;
	hdr.Data_ID = ntohs(hdr_raw.Data_ID);
	hdr.Count = ntohs(hdr_raw.Count);
	hdr.Data_type = hdr_raw.Data_type;
	payload.Count = ntohs(hdr_raw.Count);
	payload.Data_type = hdr_raw.Data_type;

	if(hdr.Count != 0){
		switch(RC_DataID_converter(hdr.Data_type)){
		case 1:{
//			HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
			pbuf_copy_partial(p, payload.from_uint8, hdr.Count, sizeof(RC_header_t));
			break;}
		case 2:{
			pbuf_copy_partial(p, payload.from_uint16, hdr.Count*sizeof(uint16_t), sizeof(RC_header_t));
			for(uint8_t i=0; i<hdr.Count;i++){
				payload.from_int16[i]=ntohs(payload.from_int16[i]);
			}
			break;}
		case 4:{
			pbuf_copy_partial(p, payload.from_uint32, hdr.Count*sizeof(uint32_t), sizeof(RC_header_t));
			for(uint8_t i=0; i<hdr.Count;i++){
				payload.from_int32[i]=ntohl(payload.from_int32[i]);
						}
			break;}
		case 8:{
			pbuf_copy_partial(p, payload.from_double, hdr.Count*sizeof(double), sizeof(RC_header_t));
			for(uint8_t i=0; i<hdr.Count;i++){
				payload.from_double[i]=ntohxl(payload.from_double[i]);
						}
			break;}
		}}


	pbuf_free(p); //clears buff for new messages

	switch(hdr.Data_ID){
	case 1:{
		RC_CAN_send(submodule->CAN_ID, 1,0,0);
		break;}

	}



}
//
//void submodule_add_subscriber(const ip_addr_t *addr){
//	//check for duplicate
//	for (int i =0; i<maxSubscriber; i++){
//		if (ip_addr_cmp(&substribers[i], addr)){
//			return;
//		}
//	}
//
//	//find open spot
//	for (int i=0; i<maxSubscriber; i++){
//		if(ip_addr_cmp(&substribers[i], &Zero_IP)){
//			ip_addr_copy(substribers[i], *addr);
//
//			return;
//		}
//	}
//
//
//}
//void submodule_remove_subscriber(const ip_addr_t *addr){
//	//searches for IP in array
//	for (int i =0; i<maxSubscriber; i++){
//			if (ip_addr_cmp(&substribers[i], addr)){
//				ip_addr_copy(substribers[i], Zero_IP);
//
//
//			}
//		}
//
//
//}
//
//void submodule_Send_subscriber(uint16_t data_id, uint16_t data_length, uint8_t data_size,uint8_t data_type, const void *data){
//
//	size_t pack_bytes =sizeof(RC_header_t)+(data_length*data_size);
//		//creation of the buffer being sent
//		struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (uint16_t)pack_bytes, PBUF_RAM);
//
//	RC_header_t hdr;
//		hdr.version =0x19;
//		hdr.Data_ID = htons(data_id);
//		hdr.Count = htons(data_length);
//		hdr.Data_type = data_type;
//		//adding the header into the buffer no payload for this send
//		memcpy((uint8_t *)p->payload, &hdr, sizeof(RC_header_t));
//
//		//add payload
//
//
//
//
//		if(data_length !=0){
//		switch(data_size){
//		case 1:
//
//			memcpy((uint8_t *)p->payload + sizeof(RC_header_t), data, data_length);
//			break;
//		case 2:{
//			uint16_t val[data_length];
//			memcpy(val, data, sizeof(uint16_t)*data_length);
//			for(uint16_t i=0; i<data_length; i++){
//				val[i] = htons(val[i]);}
//			memcpy((uint8_t *)p->payload + sizeof(RC_header_t), val, data_size*data_length);
//			break;}
//		case 4:{
//			uint32_t val[data_length];
//			memcpy(val, data, sizeof(uint32_t)*data_length);
//			for(uint16_t i=0; i<data_length; i++){
//				val[i]=htonl(val[i]);}
//			memcpy((uint8_t *)p->payload + sizeof(RC_header_t), val, data_size*data_length);
//			break;}
//		case 8:{
//			uint64_t val[data_length];
//			memcpy(val, data, sizeof(uint64_t)*data_length);
//			for(uint16_t i=0; i<data_length; i++){
//				val[i]=htonxl(val[i]);}
//			memcpy((uint8_t *)p->payload + sizeof(RC_header_t), val, data_size*data_length);
//			break;}
//
//		}}
//
//		for (int i =0; i<maxSubscriber; i++){
//			if(!ip_addr_cmp(&substribers[i], &Zero_IP)){
//				udp_sendto(upcb, p, &substribers[i], RC_Port);
//		}}
//		pbuf_free(p);
//
//}
//










