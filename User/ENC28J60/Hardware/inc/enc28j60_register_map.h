#include "stdint.h"
#include  "stddef.h"

//-----------------------------------ENC28J60 REGISTER MAP---------------------------------------

//--------------------------CONFIGURATION----------------------

//Memory allocation
#define RX_BUFFER_START_ADDR	0x0
#define RX_BUFFER_END_ADDR		0x17ff
#define TX_BUFFER_START_ADDR	0x1800

//------------------------CONSTANT ADDRESSES-------------------

//SPI read/write buffer memory address
#define BUFFER_MEMORY_ARG			0x1A
#define SOFT_RESET_ARG				0x1F

//--------------------------OPCODE MASKS-----------------------

//Opcode
typedef enum{
	ReadControlRegister 	= 0x00,
	ReadBufferMemory			= 0x20,
	WriteControlRegister	= 0x40,
	WriteBufferMemory			=	0x60,
	BitFieldSet 					=	0x80,
	BitFieldClear 				=	0xA0,
	SystemResetCommand		=	0xE0,
}Opcode_t;


//------------------------REGISTER TYPEDEFS--------------------

//Bank selection
typedef enum{
	Bank0,
	Bank1,
	Bank2,
	Bank3,
	Common,
}Bank_t;

//Register type
typedef enum{
	ETH_R = 0x00,
	MAC_R,
	MII_R,	
	PHY_R,
}RegType_t;

//Register parameters
typedef struct __attribute__((packed)){
	RegType_t type;	
	Bank_t bank;
	uint8_t reg_addr;
	uint8_t length;
}Reg_t;


//--------------------------REGISTER MAP-----------------------

//Common for all bank registers
static const Reg_t EIE 			= {ETH_R, Common,	0x1b, 1};
static const Reg_t EIR 			= {ETH_R, Common, 0x1c, 1};
static const Reg_t ESTAT		=	{ETH_R, Common, 0x1d, 1};
static const Reg_t ECON2  	= {ETH_R, Common, 0x1e, 1};	//Controlled main functions (Power management, pointer/packed decrement or increment)
static const Reg_t ECON1  	= {ETH_R, Common, 0x1f, 1};	//Controlled main functions (RX/TX, DMA, bank selection)

//Bank 0 registers
static const Reg_t ERDPTL 	= {ETH_R, Bank0,	0x00, 2};	//Receive buffer current position pointer
static const Reg_t EWRPTL 	= {ETH_R, Bank0,	0x02, 2}; //Transmit buffer current position pointer
static const Reg_t ETXSTL		=	{ETH_R, Bank0,	0x04, 2};	//Transmit buffer start address
static const Reg_t ETXNDL 	= {ETH_R, Bank0,	0x06, 2};	//Transmit buffer end address
static const Reg_t ERXSTL 	= {ETH_R, Bank0,	0x08, 2}; //Receive buffer start address
static const Reg_t ERXNDL 	= {ETH_R, Bank0,	0x0a, 2};	//Receive buffer end address
static const Reg_t ERXRDPTL	= {ETH_R, Bank0,	0x0c, 2};
static const Reg_t ERXWRPTL = {ETH_R, Bank0,	0x0e, 2};	//Internal hardware copy of ERXST
static const Reg_t EDMASTL	= {ETH_R, Bank0,	0x10, 2};
static const Reg_t EDMANDL  = {ETH_R, Bank0,	0x12, 2};
static const Reg_t EDMADSTL = {ETH_R, Bank0,	0x14, 2};
static const Reg_t EDMACSL	= {ETH_R, Bank0,	0x16, 2};

//Bank 1 registers
static const Reg_t EHT0			= {ETH_R, Bank1,  0x00, 1};
static const Reg_t EHT1			= {ETH_R, Bank1,  0x01, 1};
static const Reg_t EHT2			= {ETH_R, Bank1,  0x02, 1};
static const Reg_t EHT3			= {ETH_R, Bank1,  0x03, 1};
static const Reg_t EHT4			= {ETH_R, Bank1,  0x04, 1};
static const Reg_t EHT5			= {ETH_R, Bank1,  0x05, 1};
static const Reg_t EHT6			= {ETH_R, Bank1,  0x06, 1};
static const Reg_t EHT7			= {ETH_R, Bank1,  0x07, 1};
static const Reg_t EPMM0		= {ETH_R, Bank1,  0x08, 1};
static const Reg_t EPMM1		= {ETH_R, Bank1,  0x09, 1};
static const Reg_t EPMM2		= {ETH_R, Bank1,  0x0a, 1};
static const Reg_t EPMM3		= {ETH_R, Bank1,  0x0b, 1};
static const Reg_t EPMM4		= {ETH_R, Bank1,  0x0c, 1};
static const Reg_t EPMM5		= {ETH_R, Bank1,  0x0d, 1};
static const Reg_t EPMM6		= {ETH_R, Bank1,  0x0e, 1};
static const Reg_t EPMM7		= {ETH_R, Bank1,  0x0f, 1};
static const Reg_t EPMCSL		= {ETH_R, Bank1,  0x10, 2};
static const Reg_t EPMOL		=	{ETH_R, Bank1,  0x14, 2};
static const Reg_t ERXFCON	= {ETH_R, Bank1,  0x18, 1};
static const Reg_t EPKTCNT	= {ETH_R, Bank1,  0x19, 1};

//Bank 2 registers
static const Reg_t MACON1		= {MAC_R, Bank2,	0x00, 1};
static const Reg_t MACON3		=	{MAC_R, Bank2,	0x02, 1};
static const Reg_t MACON4		= {MAC_R, Bank2,	0x03, 1};
static const Reg_t MABBIPG	=	{MAC_R, Bank2,	0x04, 1};
static const Reg_t MAIPGL		=	{MAC_R, Bank2,	0x06, 2};
static const Reg_t MACLCON1 = {MAC_R, Bank2,	0x08, 1};
static const Reg_t MACLCON2 = {MAC_R, Bank2,	0x09, 1};
static const Reg_t MAMXFLL	= {MAC_R, Bank2,	0x0a, 2};
static const Reg_t MICMD		=	{MII_R, Bank2,	0x12, 1};
static const Reg_t MIREGADR	=	{MII_R, Bank2,	0x14, 1};
static const Reg_t MIWRL		= {MII_R, Bank2,	0x16, 2};
static const Reg_t MIRDL		=	{MII_R, Bank2,	0x18, 2};

//Bank 3 registers
static const Reg_t MAADR5		= {MAC_R, Bank3,	0x00, 1};
static const Reg_t MAADR6		=	{MAC_R, Bank3,	0x01, 1};
static const Reg_t MAADR3		=	{MAC_R, Bank3,	0x02, 1};
static const Reg_t MAADR4		= {MAC_R, Bank3,	0x03, 1};
static const Reg_t MAADR1		=	{MAC_R, Bank3,	0x04, 1};
static const Reg_t MAADR2		=	{MAC_R, Bank3,	0x05, 1};
static const Reg_t EBSTSD		=	{ETH_R, Bank3,	0x06, 1};
static const Reg_t EBSTCON	= {ETH_R, Bank3,	0x07, 1};
static const Reg_t EBSTCSL	=	{ETH_R, Bank3,	0x08, 2};
static const Reg_t MISTAT		=	{MII_R, Bank3,	0x0a, 1};
static const Reg_t EREVID		=	{ETH_R, Bank3,	0x12, 1};
static const Reg_t ECOCON		=	{ETH_R, Bank3,	0x15, 1};
static const Reg_t EFLOCON	=	{ETH_R, Bank3,	0x17, 1};
static const Reg_t EPAUSL		=	{ETH_R, Bank3,	0x18, 2};

//PHY registers
static const Reg_t PHCON1		=	{PHY_R, Common, 0x00, 2};
static const Reg_t PHSTAT1	=	{PHY_R, Common, 0x01, 2};
static const Reg_t PHID1		=	{PHY_R, Common, 0x02, 2};
static const Reg_t PHID2		=	{PHY_R, Common, 0x03, 2};
static const Reg_t PHCON2		=	{PHY_R, Common, 0x10, 2};
static const Reg_t PHSTAT2	= {PHY_R, Common, 0x11, 2};
static const Reg_t PHIE			= {PHY_R, Common, 0x12, 2};
static const Reg_t PHIR			=	{PHY_R, Common, 0x13, 2};
static const Reg_t PHLCON		= {PHY_R, Common, 0x14, 2};

//-----------------------------REGISTERS-----------------------

//Ethernet status register
typedef struct __attribute__((packed)){
	uint8_t CLKRDY					: 1;
	uint8_t TXABRT					: 1;
	uint8_t RXBUSY					: 1;
	uint8_t UNIMPLEMENTED		:	1;
	uint8_t LATECOL					: 1;
	uint8_t RESERVED				: 1;
	uint8_t BUFER						: 1;
	uint8_t INT							: 1;
}ESTAT_REG;

//Ethernet receive filter control register
typedef struct __attribute__((packed)){
	uint8_t BCEN						: 1;		//Broadcast filter enable bit
	uint8_t MCEN						: 1;		//Multicast filter enable bit
	uint8_t HTEN						: 1;		//Hash table filter enable bit
	uint8_t MPEN						: 1;		//Magic packet filter enable bit
	uint8_t PMEN						:	1;		//Pattern match filter enable bit
	uint8_t CRCEN						:	1;		//Post-filter CRC check enable bit
	uint8_t ANDOR						: 1;		//AND/OR filter select bit
	uint8_t UCEN						: 1;		//Unicast filter enable bit
}ERXFCON_REG;

//MAC control register 1
typedef struct __attribute__((packed)){
	uint8_t MARXEN					: 1;		//MAC Receive enable bit
	uint8_t PASSALL					: 1;		//Pass all received frames enable bit
	uint8_t	RXPAUS					:	1;		//Pause control frame reception enable bit
	uint8_t TXPAUS					: 1;		//Pause control frame tranmission enable bit
	uint8_t RESERVED				: 1;
	uint8_t UNIMPLEMENTED		:	3;	
}MACON1_REG;

//MAC control register 3
typedef struct __attribute__((packed)){
	uint8_t FULLDPX					: 1;		//MACON1_REG full-duplex enable bit
	uint8_t FRMLNEN					: 1;		//Frame length checking enable bit
	uint8_t HFRMEN					: 1;		//Huge frame enable bit
	uint8_t PHDREN					:	1;		//Proprieary header enable
	uint8_t TXCRCEN					: 1;		//Transmit CRC enable bit
	uint8_t PADCFG					: 3;		//Automatic pad and CRC config bits
}MACON3_REG;

//ETHERNET CONTROL REGISTER 1
typedef struct __attribute__((packed)){
	uint8_t	BSEL  					:	2;		//Bank select bits
	uint8_t RXEN  					:	1;		//Receive enable bit
	uint8_t TXRTS 					:	1;		//Transmit request to send bit
	uint8_t CSUMEN					: 1;		//DMA checksum enable bit
	uint8_t DMAST 					: 1;		//DMA start and busy status bit
	uint8_t RXRST 					: 1;		//Receive logic reset bit
	uint8_t TXRST 					: 1;		//Transmit logic reset bit
}ECON1_REG;

//ETHERNET CONTROL REGISTER 2
typedef struct __attribute__((packed)){
	uint8_t UNIMPLEMENTED		: 3;	//Read as 0
	uint8_t VRPS						:	1;	//Voltage regulator power save enable bit
	uint8_t RESERVED				:	1;	//Reserved, allways keep 0
	uint8_t PWRSV						:	1;	//Power save enable bit
	uint8_t PKTDEC					:	1;	//Packet decrement bit
	uint8_t AUTOINC 				:	1;	//Automatic buffer pointer increment enable bit
}ECON2_REG;

//MISTAT: MII STATUS REGISTER
typedef struct __attribute__((packed)){
	uint8_t BUSY						:	1;	//MII management busy bit
	uint8_t SCAN						:	1;	//MII management scan  operation bit
	uint8_t NVALID					:	1;	//MII management read data not valid bit
	uint8_t RESERVED				:	1;	//Reserved, allways keep 0
	uint8_t UNIMPLEMENTED		:	4; 	//Read as 0
}MISTAT_REG_8;

//MICMD: MII COMMAND REGISTER
typedef struct __attribute__((packed)){
	uint8_t MIIRD						:	1;	//MII read enable bit
	uint8_t MIISCAN					:	1;	//MII scan enable bit
	uint8_t UNIMPLEMENTED		:	6;	//Read as 0
}MICMD_REG_8;

//PHSTAT1: PHYSICAL LAYER STATUS REGISTER 1
typedef struct __attribute__((packed)){
	uint16_t UNIMPLEMENTED	:	1;	//Read as 0
	uint16_t JBSTAT					:	1;	//PHY latching jabber status bit
	uint16_t LLSTAT					:	1;	//PHY latchink link status bit
	uint16_t UNIMPLEMENTED2 :	8;	//Read as 0
	uint16_t PHDPX					:	1;	//Half_Duplex capable bit
	uint16_t PFDPX					:	1;	//Full_Duplex capable bit
	uint16_t UNIMPLEMENTED3	:	3;	//Read as 0
}PHSTAT1_REG_16;

//PHSTAT2: PHYSICAL LAYER STATUS REGISTER 2
typedef struct __attribute__((packed)){
	uint16_t UNIMPLEMENTED	:	5;	//Read as 0;
	uint16_t PLRITY					:	1;	//Polarity status bit
	uint16_t UNIMPLEMENTED2	:	3;	//Read as 0
	uint16_t DPXSTAT				:	1;	//PHY duplex status bit
	uint16_t LSTAT					:	1;	//PHY link status bit
	uint16_t COLSTAT				:	1;	//PHY collision status bit
	uint16_t RXSTAT					:	1;	//PHY receive status bit
	uint16_t TXSTAT					:	1;	//PHY transmit status bit
	uint16_t UNIMPLEMENTED3	:	2;	//Read as 0
}PHSTAT2_REG_16;

//PHCON1: PHY control register 1
typedef struct __attribute__((packed)){
	uint16_t UNIPLEMENTED		: 6;
	uint16_t RESERVED				: 1;
	uint16_t PDPXMD					: 1;	//PHY duplex mode bit
	uint16_t UINIPLEMENTED2	: 1;
	uint16_t RESERVED2			: 1;
	uint16_t PPWRSV					:	1;	//PHY power_down bit
	uint16_t UNIMPLEMENTED3	: 2;
	uint16_t PLOOPBK				: 1;	//PHY loopback bit
	uint16_t PRST						:	1;	//PHY software reset bit
}PHCON1_REG_16;

//---------------------CONFIG PARAMETERS-----------------------

#define MACON3_NO_AUTOPAD			0x00
#define MACON3_60B_PADDED			0x01
#define MACON3_64B_PADDED			0x03
#define MACON3_AUTO_VLAN_DET	0x05
