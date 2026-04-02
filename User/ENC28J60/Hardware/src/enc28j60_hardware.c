#include "stm32f10x.h"                  // Device header
#include "enc28j60_hardware.h"

void enc28j60_hw_spi_init(void){
	/*
		The implementation used on this device supports SPI mode 0,0 only. In addition, the SPI 
		port requires that SCK be at Idle in a low state; selectable clock polarity is not supported.
	*/
  RCC->APB2ENR|=RCC_APB2ENR_IOPAEN| RCC_APB2ENR_AFIOEN;
  GPIOA->CRL&= ~(GPIO_CRL_CNF2|GPIO_CRL_CNF4|GPIO_CRL_CNF5|GPIO_CRL_CNF6|GPIO_CRL_CNF7);
  GPIOA->CRL|=GPIO_CRL_MODE2_0|GPIO_CRL_MODE4_0|GPIO_CRL_MODE5_0|GPIO_CRL_MODE6_0|GPIO_CRL_MODE7_0;
  GPIOA->CRL|=GPIO_CRL_CNF5_1|GPIO_CRL_CNF6_1|GPIO_CRL_CNF7_1;
    
  GPIOA->BSRR|=GPIO_BSRR_BS2;
  GPIOA->BSRR|=GPIO_BSRR_BS4;
        
  RCC->APB2ENR|=RCC_APB2ENR_SPI1EN;
    
    
  SPI1->CR1|=SPI_CR1_BR_2|SPI_CR1_BR_1|SPI_CR1_MSTR|SPI_CR1_SSI;
  SPI1->CR2|=SPI_CR2_SSOE;
  SPI1->CR1|=SPI_CR1_SPE;
}

#ifdef LOGGING
void enc28j60_hw_uart_init(void){
	
	RCC->APB2ENR  |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;				// GPIOA Clock ON. Alter function clock ON
    
  GPIOA->CRH		&= ~GPIO_CRH_CNF9;																// Clear CNF bit 9
	GPIOA->CRH		|= GPIO_CRH_CNF9_1;																// Set CNF bit 9 to 10 - AFIO Push-Pull
	GPIOA->CRH		|= GPIO_CRH_MODE9_0;															// Set MODE bit 9 to Mode 01 = 10MHz 

	GPIOA->CRH		&= ~GPIO_CRH_CNF10;																// Clear CNF bit 10
	GPIOA->CRH		|= GPIO_CRH_CNF10_0;															// Set CNF bit 10 to 01 HiZ
	GPIOA->CRH		&= ~GPIO_CRH_MODE10;															// Set MODE bit 10 to Mode 01 = 10MHz 
	
	// GPIOA->AFR[1]=(GPIO_AFRH_AFR9&(1<<4))|(GPIO_AFRH_AFR10&(1<<8));
	
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;														//enable rcc on USART1
  USART1->BRR = 0x271;																						//115200 bod (read the datasheet)
  USART1->CR1 = USART_CR1_UE|USART_CR1_RE|USART_CR1_TE;						//receiver and transmitter is on, UART engaged
}

void enc28j60_hw_send_log_8b(uint8_t data){
	USART1 -> DR = data;
	while((USART1 -> SR & USART_SR_TC) == 0);                       //waiting for transmit buffer empty flag 
}

void enc28j60_hw_send_log_msg(uint8_t* msg){
	while(*msg != ('\0')){
		enc28j60_hw_send_log_8b(*msg);
		msg++;
	}
}
#endif

void enc28j60_hw_cs_high(void){
	GPIOA->BSRR|=GPIO_BSRR_BS4;
}

void enc28j60_hw_cs_low(void){
	GPIOA->BSRR|=GPIO_BSRR_BR4;
}

//Used to quick read/write operation with buffer
uint8_t enc28j60_hw_send_short_cmd(uint8_t cmd){
	*(uint8_t*)&(SPI1	->DR) = cmd;																	//sent data to spi buffer
	while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));		//waiting for SPI receive buffer to fill
	while(SPI1->SR & SPI_SR_BSY);
	return SPI1->DR;	
}

void enc28j60_hw_send_data_8b(uint8_t reg, uint8_t data){
	*(uint8_t*)&(SPI1	->DR) = reg;																	//sent data to spi buffer
	while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));		//waiting for SPI receive buffer to fill
	(void)(SPI1->DR);																								//read buffer
	while(SPI1->SR & SPI_SR_BSY);
	
	
	*(uint8_t*)&(SPI1	->DR) = data;																	//sent data to spi buffer
	while((!(SPI1->SR & SPI_SR_RXNE)) || (SPI1->SR & SPI_SR_BSY));	//waiting for SPI receive buffer to fill
	(void)(SPI1->DR);																								//read buffer
	while(SPI1->SR & SPI_SR_BSY);
}

uint8_t enc28j60_hw_read_data_8b(uint8_t reg){
	*(uint8_t*)&(SPI1	->DR) = READ_MASK|reg;  
  while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));
  (void)(SPI1->DR);
	while(SPI1->SR & SPI_SR_BSY);
       
	*(uint8_t*)&(SPI1	->DR) = 0;
  while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));
	return (uint8_t)(SPI1->DR);
}

void enc28j60_hw_send_data_16b(uint8_t reg, uint16_t data){
	
	*(uint8_t*)&(SPI1	->DR) = reg;																	//send data to spi buffer
	while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));		//waiting for SPI receive buffer to fill
	(void)(SPI1->DR);																								//read buffer
	while(SPI1->SR & SPI_SR_BSY);
	
	*(uint8_t*)&(SPI1	->DR) = data >> 8;														//send first byte of data to spi buffer
	while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));		//waiting for SPI receive buffer to fill
	(void)(SPI1->DR);																								//read buffer
	while(SPI1->SR & SPI_SR_BSY);
	
	*(uint8_t*)&(SPI1	->DR) = data;																	//send last byte of data to spi buffer
	while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));		//waiting for SPI receive buffer to fill
	(void)(SPI1->DR);																								//read buffer
	while(SPI1->SR & SPI_SR_BSY);
}

uint16_t enc28j60_hw_read_data_16b(uint8_t reg){
	
	uint16_t ret;
	*(uint8_t*)&(SPI1	->DR) = READ_MASK|reg;  
  while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));
  (void)(SPI1->DR);
	while(SPI1->SR & SPI_SR_BSY);
    
	*(uint8_t*)&(SPI1	->DR) = 0;
  while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));
  ret = ((uint8_t)(SPI1->DR)) << 8;
       
	*(uint8_t*)&(SPI1	->DR) = 0;
  while(!(SPI1->SR & SPI_SR_RXNE) || (SPI1->SR & SPI_SR_BSY));
  ret = ret | (uint8_t)(SPI1->DR);
	return ret;	
}

ENC28J60_Init_Status enc28j60_hw_is_spi_on(void){
	if(SPI1->CR1 & SPI_CR1_SPE){
		return MPU_INIT_OK;
	}
	return MPU_INIT_SPI_FAILED;
}	

