#include "stdint.h"

#define SELFTEST
#define LOGGING


#define READ_MASK 					0x80	//Маска для работы по SPI


typedef enum{
	MPU_INIT_OK,
	MPU_INIT_SPI_FAILED
} ENC28J60_Init_Status;


//Инициализация переферии для работы по UART (логгирование состояния)
#ifdef LOGGING
void enc28j60_hw_uart_init(void);
void enc28j60_hw_send_log_8b(uint8_t data);
#endif

void enc28j60_hw_spi_init(void);
void enc28j60_hw_cs_high(void);
void enc28j60_hw_cs_low(void);
void enc28j60_hw_send_short_cmd(uint8_t);
void enc28j60_hw_send_data_8b(uint8_t reg, uint8_t data);
void enc28j60_hw_send_data_16b(uint8_t reg, uint16_t data);

uint8_t enc28j60_hw_read_data_8b(uint8_t reg);
uint16_t enc28j60_hw_read_data_16b(uint8_t reg);
ENC28J60_Init_Status enc28j60_hw_is_spi_on(void);
