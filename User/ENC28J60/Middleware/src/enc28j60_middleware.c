
//--------------------------------------------ENC28J60 MIDDLEWARE LEVEL---------------------------------------------------

#include "enc28j60_hardware.h"
#include "enc28j60_middleware.h"

static Bank_t current_bank = Common;

//Нужен тестик на работу записи и чтения всех регистров
void mw_test(void){
	#ifdef LOGGING
		enc28j60_hw_uart_init();

		uint8_t passed[] = "Test passed\n";
		uint8_t failed[] = "Test failed\n";
		uint8_t test1[] = " MidWare Test1: Trying to write and read the ETH register\n";
		uint8_t test2[] = "MidWare Test2: Trying to write and read the MAC register\n";
		uint8_t test3[] = "MidWare Test3: Trying to write and read the PHY register\n";
	  uint8_t test4[] = "MidWare Test4: Trying to write and read buffer memory\n";
	
	#endif
	
	uint16_t answer = 0;
	uint16_t test_val = 0x1234;
	
	//Test1
		
	enc28j60_write_reg(&ERXSTL,test_val);		
	answer = enc28j60_read_reg(&ERXSTL);
	
	#ifdef LOGGING
		enc28j60_hw_send_log_msg(test1);
		if(answer != test_val){
			enc28j60_hw_send_log_msg(failed);
			while(1){}
		}
		enc28j60_hw_send_log_msg(passed);
	#else
		if(answer != test_val){
			while(1){}
		}		
	#endif
	
	//Test2

	do{
		answer = enc28j60_read_reg(&ESTAT);		
	}
	while(!(((ESTAT_REG*)(&answer))->CLKRDY));
		
	enc28j60_write_reg(&MAMXFLL,test_val);
	answer = enc28j60_read_reg(&MAMXFLL);
	
	#ifdef LOGGING
		enc28j60_hw_send_log_msg(test2);
		if(answer != test_val){
			enc28j60_hw_send_log_msg(failed);
			while(1){}
		}
		enc28j60_hw_send_log_msg(passed);
	#else
		if(answer != test_val){
			while(1){}
		}
	#endif
	
	//Test3
		
	uint16_t val_of_PHLCON_to_led_on = 0x3883;
	enc28j60_write_reg(&PHLCON, val_of_PHLCON_to_led_on);
	answer = enc28j60_read_reg(&PHLCON);
		
	#ifdef LOGGING
		enc28j60_hw_send_log_msg(test3);
		if((answer >> 1) != (val_of_PHLCON_to_led_on >> 1)){ 
			enc28j60_hw_send_log_msg(failed);
			while(1){}
		}
		enc28j60_hw_send_log_msg(passed);
	#else
		if((answer >> 1) != (val_of_PHLCON_to_led_on >> 1)){ 
			while(1){}
		}
	#endif
		
	//Test4
		
	uint8_t test_vals[4] = {0x12, 0x34, 0x56, 0x78};
	uint8_t returned_vals[4] = {0};
	
	//set address autoinc
	answer = enc28j60_read_reg(&ECON2);
	((ECON2_REG*)(&answer))->AUTOINC = 1; 
	enc28j60_write_reg(&ECON2, answer);
	
	//configure fifo buffer
	enc28j60_write_reg(&ERXSTL,0x00);
	enc28j60_write_reg(&ERXNDL,0xfff);

	//set the pointer to a place to write
	enc28j60_write_reg(&EWRPTL,0x1000);
	enc28j60_write_buf(test_vals, 4);
	
	//set the pointer to a place to read
	enc28j60_write_reg(&ERDPTL,0x1000);
	enc28j60_read_buf(returned_vals, 4);
	
	#ifdef LOGGING
		enc28j60_hw_send_log_msg(test4);
		for(uint8_t i = 0; i < 4; i++){
			if(test_vals[i] != returned_vals[i]){
				enc28j60_hw_send_log_msg(failed);
				while(1){}
			}
		}
		enc28j60_hw_send_log_msg(passed);
	#else
		for(uint8_t i = 0; i < 4; i++){
			if(test_vals[i] != returned_vals[i]){
				while(1){}
			}
		}
	#endif
	
	//Reset after test to prevent untracted mistakes
	enc28j60_soft_reset();
}

void enc28j60_soft_reset(void){
	enc28j60_hw_cs_low();
	//After a System Reset, all PHY registers should not be read or written to until at least 50 μs have passed since the Reset has ended. 
	enc28j60_hw_send_short_cmd(SystemResetCommand | SOFT_RESET_ARG);
	enc28j60_hw_cs_high();		
}

void enc28j60_bank_changing(Bank_t bank){
	uint8_t tmp;
	enc28j60_hw_cs_low();
	tmp =(enc28j60_hw_read_data_8b(ReadControlRegister |ECON1.reg_addr));
	((ECON1_REG*)(&tmp))->BSEL = bank;
	enc28j60_hw_cs_high();
	enc28j60_hw_cs_low();
	enc28j60_hw_send_data_8b(WriteControlRegister |ECON1.reg_addr, tmp);
	enc28j60_hw_cs_high();	
	current_bank = bank;
}

void enc28j60_write_reg(const Reg_t* reg, uint16_t value){
	if((current_bank != reg -> bank) && (reg -> bank != Common)){
		enc28j60_bank_changing(reg -> bank);
	}
	enc28j60_hw_cs_low(); 
	switch(reg->type){
		case ETH: case MAC: case MII:{
			enc28j60_hw_send_data_8b( WriteControlRegister | reg->reg_addr, value & 0xff);
			if(reg->length == 2){
				enc28j60_hw_cs_high();
				enc28j60_hw_cs_low(); 
				enc28j60_hw_send_data_8b( WriteControlRegister | (reg->reg_addr + 1), value >> 8);
			}
			break;
		}
		case PHY:
			//Write the address of the PHY register to write to into the MIREGADR register.
			enc28j60_hw_send_data_8b( WriteControlRegister | MIREGADR.reg_addr, reg->reg_addr);
		  enc28j60_hw_cs_high();
		  enc28j60_hw_cs_low(); 
			//Write the lower 8 bits of data to write into the MIWRL register
			enc28j60_hw_send_data_8b( WriteControlRegister | MIWRL.reg_addr, value & 0xff);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low(); 
			//Write the upper 8 bits of data to write into the MIWRH register.
			enc28j60_hw_send_data_8b( WriteControlRegister | (MIWRL.reg_addr + 1), value >> 8);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low(); 
			//Writing to this register automatically begins the MIIM transaction, so it must be written to after MIWRL. 
			//The MISTAT.BUSY bit becomes set.
			uint16_t state;
			do{
				state = enc28j60_read_reg(&MISTAT);
				enc28j60_hw_cs_high();
				enc28j60_hw_cs_low(); 
			//When the write operation has completed, the BUSY bit will clear itself
			}while(((MISTAT_REG_8*)&state)->BUSY == 1);
		break;
	}
	enc28j60_hw_cs_high();
}

uint16_t enc28j60_read_reg(const Reg_t* reg){
	uint16_t ret = 0;
	if((current_bank != reg -> bank) && (reg -> bank != Common)){
		enc28j60_bank_changing(reg -> bank);
	}
	enc28j60_hw_cs_low();
	switch (reg->type){
		case ETH:{
			ret = enc28j60_hw_read_data_8b(ReadControlRegister | reg->reg_addr);	
			if(reg->length == 2){
				enc28j60_hw_cs_high();
				enc28j60_hw_cs_low();
				//Get an additional high byte
				ret = ret | ((enc28j60_hw_read_data_8b(ReadControlRegister | (reg->reg_addr + 1))) << 8);	
			}
			break;
		}
		case MAC: case MII:{
			//For MAC and MII registers first readed byte is dummy and skipped
			ret = (enc28j60_hw_read_data_16b(ReadControlRegister | reg->reg_addr)) & 0xff;			
			if(reg->length == 2){
				enc28j60_hw_cs_high();
				enc28j60_hw_cs_low();
				ret = ret | (enc28j60_hw_read_data_16b(ReadControlRegister | (reg->reg_addr + 1)) << 8);
			}
			break;
		}
		case PHY:{
			enc28j60_bank_changing(MIREGADR.bank);
			enc28j60_hw_cs_low();
			
			//Previously read the MICMD register
			uint8_t tmp = 0;
			tmp = enc28j60_hw_read_data_16b(ReadControlRegister | MICMD.reg_addr);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();			
			
			//Write the address of the PHY register to read from into the MIREGADR register.
			enc28j60_hw_send_data_8b(WriteControlRegister | MIREGADR.reg_addr, reg->reg_addr);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();	

			//Set the MICMD.MIIRD bit. The read operation begins and the MISTAT.BUSY bit is set.
			((MICMD_REG_8*)(&tmp))->MIIRD = 1;
			enc28j60_hw_send_data_8b(WriteControlRegister | MICMD.reg_addr, tmp);
			enc28j60_hw_cs_high();
			
			
			for(int i = 0; i < 200; i++){ 
				__nop();
			}
			
			uint16_t state;
			enc28j60_bank_changing(MISTAT.bank);
			
			//When the MAC has obtained the register contents, the BUSY bit will clear itself.
			do{
				enc28j60_hw_cs_low();
				state = enc28j60_hw_read_data_16b(ReadControlRegister | MISTAT.reg_addr);
				enc28j60_hw_cs_high();
			}
			while(((MISTAT_REG_8*)&state)->BUSY == 1);
			
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();		
			
			//Return to bank2
			enc28j60_bank_changing(MIREGADR.bank);
			enc28j60_hw_cs_low();
			
			//Again read the MICMD register
			tmp = enc28j60_hw_read_data_16b(ReadControlRegister | MICMD.reg_addr);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();	

			//Clear the MICMD.MIIRD bit.
			((MICMD_REG_8*)(&tmp))->MIIRD = 0;
			enc28j60_hw_send_data_8b(WriteControlRegister | MICMD.reg_addr, tmp);
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();		
			
			//Read the desired data from the MIRDL and MIRDH registers. The order that these bytes are accessed is unimportant.
			ret = (enc28j60_hw_read_data_16b(ReadControlRegister | MIRDL.reg_addr)) & 0xff;
			enc28j60_hw_cs_high();
			enc28j60_hw_cs_low();	
			ret = ret | (enc28j60_hw_read_data_16b(ReadControlRegister | (MIRDL.reg_addr) + 1) << 8);
			break;
		}
		default: break;		
	}	
	enc28j60_hw_cs_high();
	return ret;
}
//Attention!!! Function works only if AUTOINC set in the ECON2 register
void enc28j60_write_buf(uint8_t* buf, uint16_t buf_lng){
	enc28j60_hw_cs_low();
	//Send comand to write data in the TX buffer
	enc28j60_hw_send_short_cmd( WriteBufferMemory | BUFFER_MEMORY_ARG);
	for(int16_t i = 0; i < buf_lng; i++){
		enc28j60_hw_send_short_cmd(*(buf + i));
	}
	enc28j60_hw_cs_high();
}

uint16_t enc28j60_read_buf(uint8_t *buf, uint16_t length){
	enc28j60_hw_cs_low();
	//Send comand to read data from the RX buffer
	enc28j60_hw_send_short_cmd( ReadBufferMemory | BUFFER_MEMORY_ARG);
	for(uint16_t i = 0; i < length; i++){
		//Reading via short comand, parameter of short_cmd have not mean
		*(buf + i) = enc28j60_hw_send_short_cmd(0xFF);
	}
	enc28j60_hw_cs_high();
}

void enc28j60_bitfield_set(const Reg_t* reg, uint8_t mask){
	if(reg->type == ETH){
		if((current_bank != reg -> bank) && (reg -> bank != Common)){
			enc28j60_bank_changing(reg -> bank);
		}
		enc28j60_hw_cs_low();
		enc28j60_hw_send_data_8b( BitFieldSet | reg->reg_addr, mask);
		//The BFS operation is terminated by raising the CS pin.
		enc28j60_hw_cs_high();
	}	
}

void enc28j60_bitfield_clear(const Reg_t* reg, uint8_t mask){
	if(reg->type == ETH){
		if((current_bank != reg -> bank) && (reg -> bank != Common)){
			enc28j60_bank_changing(reg -> bank);
		}
		enc28j60_hw_cs_low();
		enc28j60_hw_send_data_8b( BitFieldClear | reg->reg_addr, mask);\
		//The BFC operation is terminated by raising the CS pin.
		enc28j60_hw_cs_high();
	}	
}

void enc28j60_mid_init(void){
	
	//Initialize SPI communication
  enc28j60_hw_spi_init();	
	
	//Return ethernet registers to initial state
	enc28j60_soft_reset();
	
	#ifdef MW_TEST
		mw_test();
	#endif
	
	uint16_t status = 0;
	
	//Before receiving any packets, the receive buffer must be initialized by programming the ERXST and ERXND Pointers.
	//No explicit action is required to initialize the transmission buffer.
	enc28j60_write_reg(&ERXSTL,RX_BUFFER_START_ADDR);
	enc28j60_write_reg(&ERXNDL,RX_BUFFER_END_ADDR);
	
	//For tracking purposes, the ERXRDPT registers should additionally be programmed with the same value.
	enc28j60_write_reg(&ERXRDPTL, RX_BUFFER_START_ADDR);
	
	//The appropriate receive filters should be enabled or disabled by writing to the ERXFCON register.
	//Trying to start without filters
	//..............................................................
	
	//If ECON2.AUTOINC is set, it will be able to sequentially read the entire packet without ever modifying the ERDPT registers.
	status = enc28j60_read_reg(&ECON2);
	((ECON2_REG*)(&status))->AUTOINC = 1; 
	enc28j60_write_reg(&ECON2, status);
	
	/*If the initialization procedure is being executed immediately following a Power-on Reset, the ESTAT.CLKRDY
	bit should be polled to make certain that enough time has elapsed before proceeding to modify the MAC and
	PHY registers.
	*/
	do{
		status = enc28j60_read_reg(&ESTAT);		
	}
	while(!(((ESTAT_REG*)(&status))->CLKRDY));
	
	/*Set the MARXEN bit in MACON1 to enable the MAC to receive frames. If using full duplex, 
	most applications should also set TXPAUS and RXPAUS to allow IEEE defined flow control to function.
	*/
	status = enc28j60_read_reg(&MACON1);
	((MACON1_REG*)(&status))->MARXEN = 1;
	((MACON1_REG*)(&status))->TXPAUS = 1;
	((MACON1_REG*)(&status))->RXPAUS = 1;
	enc28j60_write_reg(&MACON1, status);
	
	/*Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3. Most applications should enable automatic padding to at least
	60 bytes and always append a valid CRC. For convenience, many applications may wish to set the FRMLNEN bit as well 
	to enable frame length status reporting. 
	The FULDPX bit should be set if the application will be connected to a full-duplex configured remote node; otherwise, it
	should be left clear.
	*/
	status = enc28j60_read_reg(&MACON3);
	((MACON3_REG*)(&status))->PADCFG = MACON3_64B_PADDED;
	((MACON3_REG*)(&status))->TXCRCEN = 1;
	((MACON3_REG*)(&status))->FRMLNEN = 1;
	((MACON3_REG*)(&status))->FULLDPX = 1;
	enc28j60_write_reg(&MACON3, status);
		
	//Configure the bits in MACON4. For conformance to the IEEE 802.3 standard, set the DEFER bit.
	//We will using full duplex, so setting of MACON4 is not needed
	//.................................................................
	
	/*Program the MAMXFL registers with the maximum frame length to be permitted to be received or transmitted. 
	Normal network nodes are designed to handle packets that are 1518 bytes or less.
	*/
	enc28j60_write_reg(&MAMXFLL, 1518);
	
	/*Configure the Back-to-Back Inter-Packet Gap register, MABBIPG. 
	Most applications will program this register with 15h when Full-Duplex mode is used 
	and 12h when Half-Duplex mode is used.
	*/
	enc28j60_write_reg(&MABBIPG, 0x15);
	
	//Program the local MAC address into the MAADR1:MAADR6 registers.
	#warning TODO: MAC as a macros in separate config file or in function init parameter
	enc28j60_write_reg(&MAADR1, 0x02);
	enc28j60_write_reg(&MAADR2, 0x00);
	enc28j60_write_reg(&MAADR3, 0x00);
	enc28j60_write_reg(&MAADR4, 0x00);
	enc28j60_write_reg(&MAADR5, 0x00);
	enc28j60_write_reg(&MAADR6, 0x01);
	
	//Manually set PHCON1.PDPXMD register to set the full duplex mode
	status = enc28j60_read_reg(&PHCON1);
	((PHCON1_REG_16*)(&status))->PDPXMD = 1;
	enc28j60_write_reg(&PHCON1, status);
	
	#ifdef LOGGING
		uint8_t init_copleted[] = "ECN28J60 init completed\n";
		enc28j60_hw_send_log_msg(init_copleted);
	#endif
}
