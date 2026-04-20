#include "stdint.h"
#include "enc28j60_register_map.h"
#ifndef __ENC_PRIV
#define __ENC_PRIV
typedef struct{
	uint16_t next_packet_ptr;
	uint16_t byte_count;
	uint16_t status;
}ETH_rx_header_t;

typedef struct{
    //#ifdef LOGGING
			void (*uart_init)(void);
			//void (*enc28j60_hw_send_log_8b)(uint8_t data);
			void (*uart_send_msg)(uint8_t* msg);
		//#endif

		void (*spi_init)(void);
		void (*cs_deselect)(void);
		void (*cs_select)(void);
		void (*send_1byte)(uint8_t reg, uint8_t data);
		void (*send_2bytes)(uint8_t reg, uint16_t data);

		uint8_t (*send_short_cmd)(uint8_t);
		uint8_t (*read_1byte)(uint8_t reg);
		uint16_t (*read_2bytes)(uint8_t reg);
} enc28j60_io_t;

struct enc28j60{
	enc28j60_io_t io;
	Bank_t bank;
	ETH_rx_header_t enc_header;
	uint16_t next_rx_packet_ptr;
	uint16_t tx_buff_ptr;
};
#endif
