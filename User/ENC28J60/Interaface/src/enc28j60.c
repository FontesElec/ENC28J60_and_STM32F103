//----------------------------------------ENC28J60 INTERFACE LEVEL---------------------------------------
#include "enc28j60.h"

#define ENC_HEADER_SIZE 6
#define ETH_FRAME_SIZE 	14
#define CRC_SIZE 				4
#define H_SIZE 					(ENC_HEADER_SIZE + ETH_FRAME_SIZE + CRC_SIZE)



//Pointer to frame payload 
uint8_t* buffer_ptr = NULL;

//Pointer to ENC header
ETH_rx_header_t* enc_header = NULL;

//Pointer to ETH header
eth_frame_t* frame_params = NULL;

//Pointer to a current frame
uint16_t c_rx_ptr = RX_BUFFER_START_ADDR;

//MAC address pointer
MAC_addr_t* MAC_addr = NULL;


void enc28j60_Init(MAC_addr_t* my_addr, uint8_t* buffer, ETH_rx_header_t* rx_header, eth_frame_t* params){
	buffer_ptr = buffer;
	enc_header = rx_header;
	frame_params = params;
	frame_params->data_ptr = buffer;
	MAC_addr = my_addr;
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

void processing(void){
	//Set pointer to tx start writing
	enc28j60_write_reg(&EWRPTL, TX_BUFFER_START_ADDR);
	
	uint8_t* tmp_ptr = buffer_ptr;
	
	//Write control byte
	*tmp_ptr = 0x00;
	tmp_ptr++;
	
	//Write dest MAC
	for(int i = 0; i < 6; i++){
		*tmp_ptr = *((uint8_t*)(&frame_params->source_addr) + i);
		tmp_ptr++;
	}
	
	//Write source MAC
	for(int i = 0; i < 6; i++){
		*tmp_ptr = *((uint8_t*)(MAC_addr) + i);
		tmp_ptr++;
	}
	
	//Write ETH ARP type
	//uint16_t tmp16 = 0x0806;
	*tmp_ptr = 0x08;
	tmp_ptr++;
	*tmp_ptr = 0x06;
	tmp_ptr++;
	
	//Hardware type - eternet
	//tmp16 = 0x0001;
	*tmp_ptr = 0x00;
	tmp_ptr++;
	*tmp_ptr = 0x01;
	tmp_ptr++;
	
	//Protocol type - IPv4
	//tmp16 = 0x0800;
	*tmp_ptr = 0x08;
	tmp_ptr++;
	*tmp_ptr = 0x00;
	tmp_ptr++;
	
	//Hardware size
	//uint8_t tmp8 = 0x06;
	*tmp_ptr = 0x06;
	tmp_ptr++;
	
	//Protocol size
	//tmp8 = 0x04;
	*tmp_ptr = 0x04;
	tmp_ptr++;
	
	//Opcode - reply
	//tmp16 = 0x0002;
	*tmp_ptr = 0x00;
	tmp_ptr++;
	*tmp_ptr = 0x02;
	tmp_ptr++;;
	
	//Sender MAC
	for(int i = 0; i < 6; i++){
		*tmp_ptr = *((uint8_t*)(MAC_addr) + i);
		tmp_ptr++;
	}
	
	//Sender IP
	uint8_t ip[] = {169, 254, 25, 63};
	for(int i = 0; i < 4; i++){
		*tmp_ptr = *(ip + i);
		tmp_ptr++;
	}
	
	//Target MAC
	uint8_t t_mac[] = {0xe8, 0x9c, 0x25, 0x92,0xab, 0x8e};
	for(int i = 0; i < 6; i++){
		*tmp_ptr = *(t_mac + i);
		tmp_ptr++;
	}
	
	//Target IP
	uint8_t t_ip[] = {169,254,25,10};
	for(int i = 0; i < 4; i++){
		*tmp_ptr = *(t_ip + i);
		tmp_ptr++;
	}
	
	//Send to buffer
	enc28j60_write_buf(buffer_ptr, 43);
	
	
	//Send of frame ptr
	enc28j60_write_reg(&ETXNDL, TX_BUFFER_START_ADDR + 42);
	enc28j60_write_reg(&ETXSTL, TX_BUFFER_START_ADDR);
	
	//Set to write flag
	uint16_t tmp16 = enc28j60_read_reg(&ECON1);
	((ECON1_REG*)(&tmp16))->TXRTS = 1;
	enc28j60_write_reg(&ECON1, tmp16);	
	
	//Wait to the end of writing process
	do{
		tmp16 = enc28j60_read_reg(&ECON1);
	}while(((ECON1_REG*)(&tmp16))->TXRTS == 1);
	
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
		
		//Get ETH header
		enc28j60_read_ETH_header(frame_params);
		
		if(frame_params->etype == E_TYPE_ARP){
			frame_params->payload_val = frame_length;
			enc28j60_read_payload(buffer_ptr, frame_length);
			if(buffer_ptr[24] == 169 && buffer_ptr[25] == 254 && buffer_ptr[26] == 25 && buffer_ptr[27] == 63){
				processing();
			}
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
