
//--------------------------------------------ENC28J60 MIDDLEWARE LEVEL---------------------------------------------------

#include "enc28j60_hardware.h"
#include "enc28j60_middleware.h"

static Bank_t current_bank = Common;

//Нужен тестик на работу записи и чтения всех регистров
#warning TODO: TESTS FOR HARD AND MIDDLE

void mw_test(void){
	#ifdef LOGGING
		enc28j60_hw_uart_init();

		uint8_t passed[] = "Test passed\n";
		uint8_t failed[] = "Test failed\n";
		uint8_t test1[] = " MidWare Test1: Trying to write and read the ETH register\n";
		uint8_t test2[] = "MidWare Test2: Trying to write and read the MAC register\n";
		uint8_t test3[] = "MidWare Test3: Trying to write and read the PHY register\n";
	
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
		#warning TODO: NEED TO CHECK
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

void enc28j60_bitfield_set(const Reg_t* reg, uint8_t mask){
	if(reg->type == ETH){
		if((current_bank != reg -> bank) && (reg -> bank != Common)){
			enc28j60_bank_changing(reg -> bank);
		}
		enc28j60_hw_cs_low();
		enc28j60_hw_send_data_8b( BitFieldSet | reg->reg_addr, mask);\
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
	
	//Before receiving any packets, the receive buffer must be initialized by programming the ERXST and ERXND Pointers.
	//No explicit action is required to initialize the transmission buffer.
	enc28j60_write_reg(&ERXSTL,RX_BUFFER_START_ADDR);
	enc28j60_write_reg(&ERXNDL,RX_BUFFER_END_ADDR);

	//The appropriate receive filters should be enabled or disabled by writing to the ERXFCON register.
	//Trying to start without filters
	
	
	#warning TODO: NEED TO SET AUTOINC BIT IN ECON2
	
}
