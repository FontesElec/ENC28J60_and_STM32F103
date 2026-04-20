#include "stdint.h"
#include "enc28j60_ll.h"
#include "enc28j60_priv.h"

//----------------------------------------ENC28J60 INTERFACE LEVEL---------------------------------------
typedef struct enc28j60 enc28j60_t;

//typedef struct{
//	MAC_addr_t 	dest_addr;
//	MAC_addr_t 	source_addr;
//	uint16_t 		etype;
//	uint8_t*    data_ptr;
//	uint16_t		payload_val;	
//}eth_frame_t;

//typedef struct{
//	uint16_t next_packet_ptr;
//	uint16_t byte_count;
//	uint16_t status;
//}ETH_rx_header_t;


//void enc28j60_Init(MAC_addr_t* my_addr, uint8_t* buffer, ETH_rx_header_t* rx_header, eth_frame_t* params);
void enc28j60_init(enc28j60_t *dev, MAC_addr_t* my_addr);
void enc28j60_read_rx_header(enc28j60_t *dev);
//void enc28j60_read_ETH_header(eth_frame_t* params);
void enc28j60_read_payload(enc28j60_t *dev, uint8_t* p_data, uint16_t size); 
uint16_t enc28j60_receive_packet(enc28j60_t *dev, uint8_t *buf, uint16_t maxlen);
void enc28j60_send_packet(enc28j60_t *dev, uint8_t* buf, uint16_t length);
void enc28j60_prepare_to_tx(enc28j60_t *dev);
void enc28j60_write_to_tx(enc28j60_t *dev, uint8_t* buf, uint16_t length);
void enc28j60_ready_to_send(enc28j60_t *dev);
