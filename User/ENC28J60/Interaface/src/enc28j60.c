//----------------------------------------ENC28J60 INTERFACE LEVEL---------------------------------------
#include "enc28j60.h"

#define ENC_HEADER_SIZE 6
#define ETH_FRAME_SIZE 	14
#define CRC_SIZE 				4
#define H_SIZE 					(ENC_HEADER_SIZE + ETH_FRAME_SIZE + CRC_SIZE)

#define E_TYPE_ARP  		0x0608

//Pointer to frame payload 
uint8_t* buffer_ptr = NULL;

//Pointer to ENC header
ETH_rx_header_t* enc_header = NULL;

//Pointer to ETH header
eth_frame_t* frame_params = NULL;

//Pointer to a current frame
uint16_t c_rx_ptr = RX_BUFFER_START_ADDR;


void enc28j60_Init(MAC_addr_t* my_addr, uint8_t* buffer, ETH_rx_header_t* rx_header, eth_frame_t* params){
	buffer_ptr = buffer;
	enc_header = rx_header;
	frame_params = params;
	frame_params->data_ptr = buffer;
	enc28j60_mid_init(my_addr);
};

void enc28j60_read_rx_header(ETH_rx_header_t* p_rx_header){
		enc28j60_read_buf((uint8_t*)p_rx_header, ENC_HEADER_SIZE);
}

void enc28j60_read_ETH_header(eth_frame_t* params){
		enc28j60_read_buf((uint8_t*)params, ETH_FRAME_SIZE);
}

void enc28j60_read_payload(uint8_t* p_data, uint16_t size){
		enc28j60_read_buf(p_data, size);
}

void polling_eth(void){
	//Checking new received frames
	while(enc28j60_read_reg(&EPKTCNT)){
		
		enc28j60_write_reg(&ERDPTL, c_rx_ptr);
		
		enc28j60_read_rx_header(enc_header);
		
		//Payload value calculation
		uint16_t frame_length = 0;
		if(enc_header->next_packet_ptr > c_rx_ptr){
			frame_length = enc_header->next_packet_ptr - c_rx_ptr - RX_BUFFER_START_ADDR - H_SIZE;
		}
		else{
			frame_length = RX_BUFFER_END_ADDR - (RX_BUFFER_START_ADDR << 1) - c_rx_ptr + enc_header->next_packet_ptr - H_SIZE;
		}
		frame_params->payload_val = frame_length;
		
		//Get ETH header
		enc28j60_read_ETH_header(frame_params);
		
		if(frame_params->etype == E_TYPE_ARP){
			enc28j60_read_payload(buffer_ptr, frame_length);
			while(1){}
		}
		
		//Move forward pointers
		c_rx_ptr = enc_header->next_packet_ptr;
		enc28j60_write_reg(&ERDPTL, c_rx_ptr);
		enc28j60_write_reg(&ERXRDPTL, enc_header->next_packet_ptr - 1);
		
		//Decrement the counter
		#warning TODO: need to made via bitset
		uint8_t reg_data = enc28j60_read_reg(&ECON2);
		((ECON2_REG*)(&reg_data))->PKTDEC = 1;
		enc28j60_write_reg(&ECON2, reg_data);
		
	}
}
