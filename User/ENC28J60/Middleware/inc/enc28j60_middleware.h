
//--------------------------------------------ENC28J60 MIDDLEWARE LEVEL---------------------------------------------------

#include "stdint.h"
#include "enc28j60_register_map.h"

typedef struct{
	uint8_t addr1;
	uint8_t addr2;
	uint8_t addr3;
	uint8_t addr4;
	uint8_t addr5;
	uint8_t addr6;	
}MAC_addr_t;

#define MW_TEST

/**
	After a System Reset, all PHY registers should not be read or written to until at least 50 μs have passed since the Reset has ended. 
	All registers will revert to their Reset default values. The dual port buffer memory will maintain state throughout the System Reset.
*/
void enc28j60_soft_reset(void);

/**
	Need to change current bank setting in the ECON1 register
	@param bank - number of registers group what we need
*/
void enc28j60_bank_changing(Bank_t bank);

/**
	Read command, allow to read data from ETH, MII, MAC and PHY registers
	@param 	reg - pointer to the required register
	@return value - what register contains
*/
uint16_t enc28j60_read_reg(const Reg_t* reg);


/**
	The Write Control Register (WCR) command allows the host controller to write to any of the ETH, MAC 
	and MII Control registers in any order.
	@param reg - pointer to the required register
	@param value - what need to write in the register
*/
void enc28j60_write_reg(const Reg_t* reg, uint16_t value);

/**
	The Bit Field Set (BFS) command is used to set up to 8 bits in any of the ETH Control registers
	@param reg - pointer to the required register
	@param mask - what need to set (logic: reg data | mask)
*/
void enc28j60_bitfield_set(const Reg_t* reg, uint8_t mask);


/**
	The Bit Field Clear (BFC) command is used to clear up to 8 bits in any of the ETH Control registers.
	@param reg - pointer to the required register
	@param mask - what need to set (logic: reg data & (~mask))
	1reg data : 11110001
	mask:				00010111
	result:			11100000
*/
void enc28j60_bitfield_clear(const Reg_t* reg, uint8_t mask);


/**
	The Write Buffer Memory (WBM) command allows the host controller to write bytes to the integrated 8-Kbyte
	transmit and receive buffer memory.
	ATTENTION!!! function may work with set AUTOINC bit in ECON2 register
	@param buf - pointer to the data to be written
	@param buf_lng - how many bytes need to be written
*/
void enc28j60_write_buf(uint8_t* buf, uint16_t buf_lng);

/**
	The Read Buffer Memory (RBM) command allows the host controller to read bytes from the integrated 8-Kbyte
	transmit and receive buffer memory.
	ATTENTION!!! function may work with set AUTOINC bit in ECON2 register
	@param buf - pointer to the place where the data will be written 
	@param length - how many bytes need to be read
*/
void enc28j60_read_buf(uint8_t *buf, uint16_t length);


/**
	Initializing the enc28j60 at full_duplex mode
*/
void enc28j60_mid_init(MAC_addr_t* my_addr);
