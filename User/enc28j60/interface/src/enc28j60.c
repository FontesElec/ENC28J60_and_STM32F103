//----------------------------------------ENC28J60 INTERFACE LEVEL---------------------------------------
#include "enc28j60_port.h"
#include "enc28j60.h"

#define ENC_HEADER_SIZE 6
#define ETH_FRAME_SIZE 	14
#define CRC_SIZE 				4
#define H_SIZE 					(ENC_HEADER_SIZE + ETH_FRAME_SIZE + CRC_SIZE)


void enc28j60_init(enc28j60_t *dev, MAC_addr_t* my_addr){
	
	//Initial settings
	dev->bank = Common;
	dev->next_rx_packet_ptr = RX_BUFFER_START_ADDR;
	dev->tx_buff_ptr =  TX_BUFFER_START_ADDR;
	
	
	//IO f_pointers
	dev->io.cs_deselect = enc28j60_hw_cs_high;
	dev->io.cs_select = enc28j60_hw_cs_low;
	dev->io.read_1byte = enc28j60_hw_read_data_8b;
	dev->io.read_2bytes = enc28j60_hw_read_data_16b;
	dev->io.send_1byte = enc28j60_hw_send_data_8b;
	dev->io.send_2bytes = enc28j60_hw_send_data_16b;
	dev->io.send_short_cmd = enc28j60_hw_send_short_cmd;
	dev->io.spi_init = enc28j60_hw_spi_init;
	dev->io.uart_init = enc28j60_hw_uart_init;
	dev->io.uart_send_msg = enc28j60_hw_send_log_msg;
	
	enc28j60_ll_init(dev, my_addr);
	
}

void enc28j60_read_rx_header(enc28j60_t *dev){
		enc28j60_read_buf(dev, (uint8_t*)(&(dev->enc_header)), ENC_HEADER_SIZE);
}

void enc28j60_read_payload(enc28j60_t *dev, uint8_t* p_data, uint16_t size){
		enc28j60_read_buf(dev, p_data, size);
}

uint16_t enc28j60_receive_packet(enc28j60_t *dev, uint8_t *buf, uint16_t maxlen){
	//Checking new received frames
	if(enc28j60_read_reg(dev, &EPKTCNT)){
		
		enc28j60_write_reg(dev, &ERDPTL, dev->next_rx_packet_ptr);
		enc28j60_read_rx_header(dev);
		
		uint16_t len = dev->enc_header.byte_count;

    if (len > 4) len -= 4; //Ignore the CRC
    if (len > maxlen) len = maxlen; //ignore rest data if len more than maximum length
		
		enc28j60_read_buf(dev, buf, len);
		
		//Move forward pointers
		dev->next_rx_packet_ptr = dev->enc_header.next_packet_ptr;
		enc28j60_write_reg(dev, &ERDPTL, dev->next_rx_packet_ptr);
		if (dev->enc_header.next_packet_ptr == RX_BUFFER_START_ADDR){
			enc28j60_write_reg(dev, &ERXRDPTL, RX_BUFFER_END_ADDR);
		}
		else{
			enc28j60_write_reg(dev, &ERXRDPTL, dev->enc_header.next_packet_ptr - 1);
		}
		
		//Decrement the counter
		#warning TODO: need to made via bitset
		uint8_t reg_data = enc28j60_read_reg(dev, &ECON2);
		((ECON2_REG*)(&reg_data))->PKTDEC = 1;
		enc28j60_write_reg(dev, &ECON2, reg_data);
		return len;
	}
	return 0;
}

void enc28j60_send_packet(enc28j60_t *dev, uint8_t* buf, uint16_t length){
	//Set pointer to tx start writing
	enc28j60_write_reg(dev, &EWRPTL, TX_BUFFER_START_ADDR);
	
	//Send to buffer
	enc28j60_write_buf(dev, buf, length);
	
	//Wait if the transmitting in process
	uint16_t tmp;
	do{
    tmp = enc28j60_read_reg(dev, &ECON1);
	}while(((ECON1_REG*)&tmp)->TXRTS);
	
	//Send of frame ptr
	enc28j60_write_reg(dev, &ETXNDL, TX_BUFFER_START_ADDR + length - 1);
	enc28j60_write_reg(dev, &ETXSTL, TX_BUFFER_START_ADDR);
	
	//Set to write flag
	uint16_t tmp16 = enc28j60_read_reg(dev, &ECON1);
	((ECON1_REG*)(&tmp16))->TXRTS = 1;
	enc28j60_write_reg(dev, &ECON1, tmp16);	
	
	//Wait to the end of writing process
	do{
		tmp16 = enc28j60_read_reg(dev, &ECON1);
	}while(((ECON1_REG*)(&tmp16))->TXRTS == 1);
}

void enc28j60_write_to_tx(enc28j60_t *dev, uint8_t* buf, uint16_t length){
		//Set pointer to tx start writing
		enc28j60_write_reg(dev, &EWRPTL, dev->tx_buff_ptr);
	
		//Send to buffer
		enc28j60_write_buf(dev, buf, length);	
	
		dev->tx_buff_ptr += length;
}

void enc28j60_ready_to_send(enc28j60_t *dev){
	//Wait if the transmitting in process
	uint16_t tmp;
	do{
    tmp = enc28j60_read_reg(dev, &ECON1);
	}while(((ECON1_REG*)&tmp)->TXRTS);
	
	//Send of frame ptr
	enc28j60_write_reg(dev, &ETXNDL, dev->tx_buff_ptr - 1);
	enc28j60_write_reg(dev, &ETXSTL, TX_BUFFER_START_ADDR);
	
	
	//Set to write flag
	tmp = enc28j60_read_reg(dev, &ECON1);
	((ECON1_REG*)(&tmp))->TXRTS = 1;
	enc28j60_write_reg(dev, &ECON1, tmp);	
	
	//Wait to the end of writing process
	do{
		tmp = enc28j60_read_reg(dev, &ECON1);
	}while(((ECON1_REG*)(&tmp))->TXRTS == 1);
	
	//return tx_ptr to initial value
	dev->tx_buff_ptr = TX_BUFFER_START_ADDR;
}

void enc28j60_prepare_to_tx(enc28j60_t *dev){
	dev->tx_buff_ptr = TX_BUFFER_START_ADDR;
}
