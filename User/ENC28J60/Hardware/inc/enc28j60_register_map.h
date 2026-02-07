#include "stdint.h"
#include  "stddef.h"

//-----------------------------------ENC28J60 REGISTER MAP---------------------------------------

//--------------------------CONFIGURATION----------------------

//Memory allocation
#define RX_BUFFER_START_ADDR	0x0050
#define RX_BUFFER_END_ADDR		0x1fff

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
	ETH = 0x00,
	MAC,
	MII,	
	PHY,
}RegType_t;

//Register parameters
typedef struct __attribute__((packed)){
	RegType_t type;	
	Bank_t bank;
	uint8_t reg_addr;
	uint8_t length;
}Reg_t;


//--------------------------REGISTER MAP-----------------------

static const Reg_t EIE 			= {ETH, Common,	0x1b, 1};
static const Reg_t EIR 			= {ETH, Common, 0x1c, 1};
static const Reg_t ESTAT		=	{ETH, Common, 0x1d, 1};
static const Reg_t ECON2  	= {ETH, Common, 0x1e, 1};	//Controlled main functions (Power management, pointer/packed decrement or increment)
static const Reg_t ECON1  	= {ETH, Common, 0x1f, 1};	//Controlled main functions (RX/TX, DMA, bank selection)


static const Reg_t ERDPTL 	= {ETH, Bank0,	0x00, 2};	//Receive buffer current position pointer
static const Reg_t EWRPTL 	= {ETH, Bank0,	0x02, 2}; //Transmit buffer current position pointer
static const Reg_t ETXSTL		=	{ETH, Bank0,	0x04, 2};	//Transmit buffer start address
static const Reg_t ETXNDL 	= {ETH, Bank0,	0x06, 2};	//Transmit buffer end address
static const Reg_t ERXSTL 	= {ETH, Bank0,	0x08, 2}; //Receive buffer start address
static const Reg_t ERXNDL 	= {ETH, Bank0,	0x0a, 2};	//Receive buffer end address
static const Reg_t ERXRDPTL	= {ETH, Bank0,	0x0c, 2};
static const Reg_t ERXWRPTL = {ETH, Bank0,	0x0e, 2};	//Internal hardware copy of ERXST
static const Reg_t EDMASTL	= {ETH, Bank0,	0x10, 2};
static const Reg_t EDMANDL  = {ETH, Bank0,	0x12, 2};
static const Reg_t EDMADSTL = {ETH, Bank0,	0x14, 2};
static const Reg_t EDMACSL	= {ETH, Bank0,	0x16, 2};


static const Reg_t EHT0			= {ETH, Bank1,  0x00, 1};
static const Reg_t EHT1			= {ETH, Bank1,  0x01, 1};
static const Reg_t EHT2			= {ETH, Bank1,  0x02, 1};
static const Reg_t EHT3			= {ETH, Bank1,  0x03, 1};
static const Reg_t EHT4			= {ETH, Bank1,  0x04, 1};
static const Reg_t EHT5			= {ETH, Bank1,  0x05, 1};
static const Reg_t EHT6			= {ETH, Bank1,  0x06, 1};
static const Reg_t EHT7			= {ETH, Bank1,  0x07, 1};
static const Reg_t EPMM0		= {ETH, Bank1,  0x08, 1};
static const Reg_t EPMM1		= {ETH, Bank1,  0x09, 1};
static const Reg_t EPMM2		= {ETH, Bank1,  0x0a, 1};
static const Reg_t EPMM3		= {ETH, Bank1,  0x0b, 1};
static const Reg_t EPMM4		= {ETH, Bank1,  0x0c, 1};
static const Reg_t EPMM5		= {ETH, Bank1,  0x0d, 1};
static const Reg_t EPMM6		= {ETH, Bank1,  0x0e, 1};
static const Reg_t EPMM7		= {ETH, Bank1,  0x0f, 1};
static const Reg_t EPMCSL		= {ETH, Bank1,  0x10, 2};
static const Reg_t EPMOL		=	{ETH, Bank1,  0x14, 2};
static const Reg_t ERXFCON	= {ETH, Bank1,  0x18, 1};
static const Reg_t EPKTCNT	= {ETH, Bank1,  0x19, 1};


static const Reg_t MACON1		= {MAC, Bank2,	0x00, 1};
static const Reg_t MACON3		=	{MAC, Bank2,	0x02, 1};
static const Reg_t MACON4		= {MAC, Bank2,	0x03, 1};
static const Reg_t MABBIPG	=	{MAC, Bank2,	0x04, 1};
static const Reg_t MAIPGL		=	{MAC, Bank2,	0x06, 2};
static const Reg_t MACLCON1 = {MAC, Bank2,	0x08, 1};
static const Reg_t MACLCON2 = {MAC, Bank2,	0x09, 1};
static const Reg_t MAMXFLL	= {MAC, Bank2,	0x0a, 2};
static const Reg_t MICMD		=	{MII, Bank2,	0x12, 1};
static const Reg_t MIREGADR	=	{MII, Bank2,	0x14, 1};
static const Reg_t MIWRL		= {MII, Bank2,	0x16, 2};
static const Reg_t MIRDL		=	{MII, Bank2,	0x18, 2};


static const Reg_t MAADR5		= {MAC, Bank3,	0x00, 1};
static const Reg_t MAADR6		=	{MAC, Bank3,	0x01, 1};
static const Reg_t MAADR3		=	{MAC, Bank3,	0x02, 1};
static const Reg_t MAADR4		= {MAC, Bank3,	0x03, 1};
static const Reg_t MAADR1		=	{MAC, Bank3,	0x04, 1};
static const Reg_t MAADR2		=	{MAC, Bank3,	0x05, 1};
static const Reg_t EBSTSD		=	{ETH, Bank3,	0x06, 1};
static const Reg_t EBSTCON	= {ETH, Bank3,	0x07, 1};
static const Reg_t EBSTCSL	=	{ETH, Bank3,	0x08, 2};
static const Reg_t MISTAT		=	{MII, Bank3,	0x0a, 1};
static const Reg_t EREVID		=	{ETH, Bank3,	0x12, 1};
static const Reg_t ECOCON		=	{ETH, Bank3,	0x15, 1};
static const Reg_t EFLOCON	=	{ETH, Bank3,	0x17, 1};
static const Reg_t EPAUSL		=	{ETH, Bank3,	0x18, 2};

static const Reg_t PHCON1		=	{PHY, Common, 0x00, 2};
static const Reg_t PHSTAT1	=	{PHY, Common, 0x01, 2};
static const Reg_t PHID1		=	{PHY, Common, 0x02, 2};
static const Reg_t PHID2		=	{PHY, Common, 0x03, 2};
static const Reg_t PHCON2		=	{PHY, Common, 0x10, 2};
static const Reg_t PHSTAT2	= {PHY, Common, 0x11, 2};
static const Reg_t PHIE			= {PHY, Common, 0x12, 2};
static const Reg_t PHIR			=	{PHY, Common, 0x13, 2};
static const Reg_t PHLCON		= {PHY, Common, 0x14, 2};





//------------------------COMMON REGISTERS---------------------
//typedef enum{
//	EIE 		= 0x1b,
//	EIR,
//	ESTAT,
//	ECON2,										
//	ECON1,										
//} ENC28J60_COMMON_TypeDef;




//----------------------------BANK0 (ETHERNET BUFFER REGISTERS)----------------------------
//typedef enum{
//	ERDPTL 		= 0x00,					//Receive buffer current position pointer lower register
//	ERDPTH,										//Receive buffer current position pointer upper register
//	EWRPTL,										//Transmit buffer current position pointer lower register
//	EWRPTH,										//Transmit buffer current position pointer upper register
//	ETXSTL,										//Transmit buffer start address lower register
//	ETXSTH,										//Transmit buffer start address upper register
//	ETXNDL,										//Transmit buffer end address lower register
//	ETXNDH,										//Transmit buffer end address upper register
//	ERXSTL,										//Receive buffer start address lower register
//	ERXSTH,										//Receive buffer start address upper register
//	ERXNDL,										//Receive buffer end address lower register
//	ERXNDH,										//Receive buffer end address upper register
//	ERXRDPTL,
//	ERXRDPTH,
//	ERXWRPTL,									//Internal hardware copy of ERXST lower register (readonly)
//	ERXWRPTH,									//Internal hardware copy of ERXST upper register (readonly)
//	EDMASTL,
//	EDMASTH,
//	EDMANDL,
//	EDMANDH,
//	EDMADSTL,
//	EDMADSTH,
//	EDMACSL,
//	EDMACSH,
//} ENC28J60_BANK0_TypeDef;


//----------------------------BANK1----------------------------
//typedef enum{
//	EHT0 			= 0x00,
//	EHT1,
//	EHT2,
//	EHT3,
//	EHT4,
//	EHT5,
//	EHT6,
//	EHT7,
//	EPMM0,
//	EPMM1,
//	EPMM2,
//	EPMM3,
//	EPMM4,
//	EPMM5,
//	EPMM6,
//	EPMM7,
//	EPMCSL,
//	SPMCSH,
//	EPMOL 		= 0x14,
//	EPMOH,
//	ERXFCON 	= 0x18,
//	EPKTCNT,
//}	ENC28J60_BANK1_TypeDef;


//----------------------------BANK2----------------------------
//typedef enum{
//	MACON1 		= 0x00,
//	MACON3 		= 0x02,
//	MACON4,
//	MABBIPG,
//	MAIPGL 		= 0x06,
//	MAIPGH,
//	MACLCON1,
//	MACLCON2,
//	MAMXFLL,
//	MAMXFLH,
//	MICMD 		= 0x12,
//	MIREGADR 	= 0x14,
//	MIWRL 		= 0x16,
//	MIWRH,
//	MIRDL,
//	MIRDH,	
//}NC28J60_BANK2_TypeDef;



//----------------------------BANK3----------------------------
//typedef enum{
//	MAADR5 = 0x00,
//	MAADR6,
//	MAADR3,
//	MAADR4,
//	MAADR1,
//	MAADR2,
//	EBSTSD,
//	EBSTCON,
//	EBSTCSL,
//	EBSTCSH,
//	MISTAT,
//	EREVID = 0x12,
//	ECOCON = 0x15,
//	EFLOCON = 0x17,
//	EPAUSL,
//	EPAUSH,
//}NC28J60_BANK3_TypeDef;


//-----------------------------REGISTERS-----------------------


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

