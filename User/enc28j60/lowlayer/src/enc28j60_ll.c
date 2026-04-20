
//--------------------------------------------ENC28J60 MIDDLEWARE LEVEL---------------------------------------------------

#include "enc28j60_ll.h"


#ifdef LL_TEST
void ll_test(struct enc28j60 *dev){
	#ifdef LOGGING
		dev->io.uart_init();
		//enc28j60_hw_uart_init();

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
		
	enc28j60_write_reg(dev, &ERXSTL, test_val);		
	answer = enc28j60_read_reg(dev, &ERXSTL);
	
	#ifdef LOGGING
		dev->io.uart_send_msg(test1);
		if(answer != test_val){
			dev->io.uart_send_msg(failed);
			while(1){}
		}
		dev->io.uart_send_msg(passed);
	#else
		if(answer != test_val){
			while(1){}
		}		
	#endif
	
	//Test2

	do{
		answer = enc28j60_read_reg(dev, &ESTAT);		
	}
	while(!(((ESTAT_REG*)(&answer))->CLKRDY));
		
	enc28j60_write_reg(dev, &MAMXFLL, test_val);
	answer = enc28j60_read_reg(dev, &MAMXFLL);
	
	#ifdef LOGGING
		dev->io.uart_send_msg(test2);
		if(answer != test_val){
			dev->io.uart_send_msg(failed);
			while(1){}
		}
		dev->io.uart_send_msg(passed);
	#else
		if(answer != test_val){
			while(1){}
		}
	#endif
	
	//Test3
		
	uint16_t val_of_PHLCON_to_led_on = 0x3883;
	enc28j60_write_reg(dev, &PHLCON, val_of_PHLCON_to_led_on);
	answer = enc28j60_read_reg(dev, &PHLCON);
		
	#ifdef LOGGING
		dev->io.uart_send_msg(test3);
		if((answer >> 1) != (val_of_PHLCON_to_led_on >> 1)){ 
			dev->io.uart_send_msg(failed);
			while(1){}
		}
		dev->io.uart_send_msg(passed);
	#else
		if((answer >> 1) != (val_of_PHLCON_to_led_on >> 1)){ 
			while(1){}
		}
	#endif
		
	//Test4
		
	uint8_t test_vals[4] = {0x12, 0x34, 0x56, 0x78};
	uint8_t returned_vals[4] = {0};
	
	//set address autoinc
	answer = enc28j60_read_reg(dev, &ECON2);
	((ECON2_REG*)(&answer))->AUTOINC = 1; 
	enc28j60_write_reg(dev, &ECON2, answer);
	
	//configure fifo buffer
	enc28j60_write_reg(dev,&ERXSTL,0x00);
	enc28j60_write_reg(dev,&ERXNDL,0xfff);

	//set the pointer to a place to write
	enc28j60_write_reg(dev,&EWRPTL,0x1000);
	enc28j60_write_buf(dev, test_vals, 4);
	
	//set the pointer to a place to read
	enc28j60_write_reg(dev,&ERDPTL,0x1000);
	enc28j60_read_buf(dev, returned_vals, 4);
	
	#ifdef LOGGING
		dev->io.uart_send_msg(test4);
		for(uint8_t i = 0; i < 4; i++){
			if(test_vals[i] != returned_vals[i]){
				dev->io.uart_send_msg(failed);
				while(1){}
			}
		}
		dev->io.uart_send_msg(passed);
	#else
		for(uint8_t i = 0; i < 4; i++){
			if(test_vals[i] != returned_vals[i]){
				while(1){}
			}
		}
	#endif
	
	//Reset after test to prevent untracted mistakes
	enc28j60_soft_reset(dev);
}
#endif

void enc28j60_soft_reset(struct enc28j60 *dev){
	dev->io.cs_select();
	//After a System Reset, all PHY registers should not be read or written to until at least 50 μs have passed since the Reset has ended. 
	dev->io.send_short_cmd(SystemResetCommand | SOFT_RESET_ARG);
	dev->io.cs_deselect();
}

void enc28j60_bank_changing(struct enc28j60 *dev, Bank_t new_bank){
	uint8_t tmp;
	dev->io.cs_select();
	tmp = dev->io.read_1byte(ReadControlRegister |ECON1.reg_addr);
	((ECON1_REG*)(&tmp))->BSEL = new_bank;
	dev->io.cs_deselect();
	dev->io.cs_select();
	dev->io.send_1byte(WriteControlRegister |ECON1.reg_addr, tmp);
	dev->io.cs_deselect();
	dev->bank = new_bank; 
}

void enc28j60_write_reg(struct enc28j60 *dev, const Reg_t* reg, uint16_t value){
	if((dev->bank != reg -> bank) && (reg -> bank != Common)){
		enc28j60_bank_changing(dev, reg -> bank);
	}
	dev->io.cs_select();
	switch(reg->type){
		case ETH_R: case MAC_R: case MII_R:{
			dev->io.send_1byte(WriteControlRegister | reg->reg_addr, value & 0xff);
			if(reg->length == 2){
				dev->io.cs_deselect();
				dev->io.cs_select();
				dev->io.send_1byte(WriteControlRegister | (reg->reg_addr + 1), value >> 8);
			}
			break;
		}
		case PHY_R:
			//Write the address of the PHY register to write to into the MIREGADR register.
			dev->io.send_1byte(WriteControlRegister | MIREGADR.reg_addr, reg->reg_addr);
		  dev->io.cs_deselect();
		  dev->io.cs_select();
			//Write the lower 8 bits of data to write into the MIWRL register
			dev->io.send_1byte(WriteControlRegister | MIWRL.reg_addr, value & 0xff);
			dev->io.cs_deselect();
			dev->io.cs_select();
			//Write the upper 8 bits of data to write into the MIWRH register.
			dev->io.send_1byte(WriteControlRegister | (MIWRL.reg_addr + 1), value >> 8);
			dev->io.cs_deselect();
			dev->io.cs_select();
			//Writing to this register automatically begins the MIIM transaction, so it must be written to after MIWRL. 
			//The MISTAT.BUSY bit becomes set.
			uint16_t state;
			do{
				state = enc28j60_read_reg(dev, &MISTAT);
				dev->io.cs_deselect();
				dev->io.cs_select();
			//When the write operation has completed, the BUSY bit will clear itself
			}while(((MISTAT_REG_8*)&state)->BUSY == 1);
		break;
	}
	dev->io.cs_deselect();
}

uint16_t enc28j60_read_reg(struct enc28j60 *dev, const Reg_t* reg){
	uint16_t ret = 0;
	if((dev->bank != reg -> bank) && (reg -> bank != Common)){
		enc28j60_bank_changing(dev, reg -> bank);
	}
	dev->io.cs_select();
	switch (reg->type){
		case ETH_R:{
			ret = dev->io.read_1byte(ReadControlRegister | reg->reg_addr);
			if(reg->length == 2){
				dev->io.cs_deselect();
				dev->io.cs_select();
				//Get an additional high byte
				ret = ret | ((dev->io.read_1byte(ReadControlRegister | (reg->reg_addr + 1))) << 8);	
			}
			break;
		}
		case MAC_R: case MII_R:{
			//For MAC and MII registers first readed byte is dummy and skipped
			ret = (dev->io.read_2bytes(ReadControlRegister | reg->reg_addr)) & 0xff;
			if(reg->length == 2){
				dev->io.cs_deselect();
				dev->io.cs_select();
				ret = ret | (dev->io.read_2bytes(ReadControlRegister | (reg->reg_addr + 1)) << 8);
			}
			break;
		}
		case PHY_R:{
			enc28j60_bank_changing(dev, MIREGADR.bank);
			dev->io.cs_select();
			
			//Previously read the MICMD register
			uint8_t tmp = 0;
			tmp = dev->io.read_2bytes(ReadControlRegister | MICMD.reg_addr);
			dev->io.cs_deselect();
			dev->io.cs_select();
			
			//Write the address of the PHY register to read from into the MIREGADR register.
			dev->io.send_1byte(WriteControlRegister | MIREGADR.reg_addr, reg->reg_addr);
			dev->io.cs_deselect();
			dev->io.cs_select();

			//Set the MICMD.MIIRD bit. The read operation begins and the MISTAT.BUSY bit is set.
			((MICMD_REG_8*)(&tmp))->MIIRD = 1;
			dev->io.send_1byte(WriteControlRegister | MICMD.reg_addr, tmp);
			dev->io.cs_deselect();
			
			for(int i = 0; i < 200; i++){ 
				__nop();
			}
			
			uint16_t state;
			enc28j60_bank_changing(dev, MISTAT.bank);
			
			//When the MAC has obtained the register contents, the BUSY bit will clear itself.
			do{
				dev->io.cs_select();
				state = dev->io.read_2bytes(ReadControlRegister | MISTAT.reg_addr);
				dev->io.cs_deselect();
			}
			while(((MISTAT_REG_8*)&state)->BUSY == 1);
			
			dev->io.cs_deselect();
			dev->io.cs_select();
			
			//Return to bank2
			enc28j60_bank_changing(dev, MIREGADR.bank);
			dev->io.cs_select();
			
			//Again read the MICMD register
			tmp = dev->io.read_2bytes(ReadControlRegister | MICMD.reg_addr);
			dev->io.cs_deselect();
			dev->io.cs_select();

			//Clear the MICMD.MIIRD bit.
			((MICMD_REG_8*)(&tmp))->MIIRD = 0;
			dev->io.send_1byte(WriteControlRegister | MICMD.reg_addr, tmp);
			dev->io.cs_deselect();
			dev->io.cs_select();
			
			//Read the desired data from the MIRDL and MIRDH registers. The order that these bytes are accessed is unimportant.
			ret = dev->io.read_2bytes(ReadControlRegister | MIRDL.reg_addr) & 0xff;
			dev->io.cs_deselect();
			dev->io.cs_select();
			ret = ret | (dev->io.read_2bytes(ReadControlRegister | (MIRDL.reg_addr) + 1) << 8);
			break;
		}
		default: break;		
	}	
	dev->io.cs_deselect();
	return ret;
}
//Attention!!! Function works only if AUTOINC set in the ECON2 register
void enc28j60_write_buf(struct enc28j60 *dev, uint8_t* buf, uint16_t buf_lng){
	dev->io.cs_select();
	//Send comand to write data in the TX buffer
	dev->io.send_short_cmd(WriteBufferMemory | BUFFER_MEMORY_ARG);
	for(int16_t i = 0; i < buf_lng; i++){
		dev->io.send_short_cmd(*(buf + i));
	}
	dev->io.cs_deselect();
}

void enc28j60_read_buf(struct enc28j60 *dev, uint8_t *buf, uint16_t length){
	dev->io.cs_select();
	//Send comand to read data from the RX buffer
	dev->io.send_short_cmd(ReadBufferMemory | BUFFER_MEMORY_ARG);
	for(uint16_t i = 0; i < length; i++){
		//Reading via short comand, parameter of short_cmd have not mean
		*(buf + i) = dev->io.send_short_cmd(0xFF);
	}
	dev->io.cs_deselect();
}

void enc28j60_bitfield_set(struct enc28j60 *dev, const Reg_t* reg, uint8_t mask){
	if(reg->type == ETH_R){
		if((dev->bank != reg -> bank) && (reg -> bank != Common)){
			enc28j60_bank_changing(dev, reg -> bank);
		}
		dev->io.cs_select();
		dev->io.send_1byte(BitFieldSet | reg->reg_addr, mask);
		//The BFS operation is terminated by raising the CS pin.
		dev->io.cs_deselect();
	}	
}

void enc28j60_bitfield_clear(struct enc28j60 *dev, const Reg_t* reg, uint8_t mask){
	if(reg->type == ETH_R){
		if((dev->bank != reg -> bank) && (reg -> bank != Common)){
			enc28j60_bank_changing(dev, reg -> bank);
		}
		dev->io.cs_select();
		dev->io.send_1byte(BitFieldClear | reg->reg_addr, mask);
		//The BFC operation is terminated by raising the CS pin.
		dev->io.cs_deselect();
	}	
}

void enc28j60_ll_init(struct enc28j60 *dev, MAC_addr_t* my_addr){
	
	//Initialize SPI communication
  dev->io.spi_init();
	
	//Return ethernet registers to initial state
	enc28j60_soft_reset(dev);
	
	#ifdef LL_TEST
		ll_test(dev);
	#endif
	
	uint16_t status = 0;
	
	//Before receiving any packets, the receive buffer must be initialized by programming the ERXST and ERXND Pointers.
	//No explicit action is required to initialize the transmission buffer.
	enc28j60_write_reg(dev, &ERXSTL,RX_BUFFER_START_ADDR);
	enc28j60_write_reg(dev, &ERXNDL,RX_BUFFER_END_ADDR);
	
	//For tracking purposes, the ERXRDPT registers should additionally be programmed with the same value.
	enc28j60_write_reg(dev, &ERXRDPTL, RX_BUFFER_END_ADDR);
	
	//The appropriate receive filters should be enabled or disabled by writing to the ERXFCON register.
	status = enc28j60_read_reg(dev, &ERXFCON);
	((ERXFCON_REG*)(&status))->CRCEN = 1;
	enc28j60_write_reg(dev, &ERXFCON, status);
	
	//If ECON2.AUTOINC is set, it will be able to sequentially read the entire packet without ever modifying the ERDPT registers.
	status = enc28j60_read_reg(dev, &ECON2);
	((ECON2_REG*)(&status))->AUTOINC = 1; 
	enc28j60_write_reg(dev, &ECON2, status);
	
	/*If the initialization procedure is being executed immediately following a Power-on Reset, the ESTAT.CLKRDY
	bit should be polled to make certain that enough time has elapsed before proceeding to modify the MAC and
	PHY registers.
	*/
	do{
		status = enc28j60_read_reg(dev, &ESTAT);		
	}
	while(!(((ESTAT_REG*)(&status))->CLKRDY));
	
	/*Set the MARXEN bit in MACON1 to enable the MAC to receive frames. If using full duplex, 
	most applications should also set TXPAUS and RXPAUS to allow IEEE defined flow control to function.
	*/
	status = enc28j60_read_reg(dev, &MACON1);
	((MACON1_REG*)(&status))->MARXEN = 1;
	((MACON1_REG*)(&status))->TXPAUS = 1;
	((MACON1_REG*)(&status))->RXPAUS = 1;
	enc28j60_write_reg(dev, &MACON1, status);
	
	/*Configure the PADCFG, TXCRCEN and FULDPX bits of MACON3. Most applications should enable automatic padding to at least
	60 bytes and always append a valid CRC. For convenience, many applications may wish to set the FRMLNEN bit as well 
	to enable frame length status reporting. 
	The FULDPX bit should be set if the application will be connected to a full-duplex configured remote node; otherwise, it
	should be left clear.
	*/
	status = enc28j60_read_reg(dev, &MACON3);
	((MACON3_REG*)(&status))->PADCFG = MACON3_60B_PADDED;
	((MACON3_REG*)(&status))->TXCRCEN = 1;
	((MACON3_REG*)(&status))->FRMLNEN = 1;
	((MACON3_REG*)(&status))->FULLDPX = 1;
	enc28j60_write_reg(dev, &MACON3, status);
		
	//Configure the bits in MACON4. For conformance to the IEEE 802.3 standard, set the DEFER bit.
	//We will using full duplex, so setting of MACON4 is not needed
	//.................................................................
	
	/*Program the MAMXFL registers with the maximum frame length to be permitted to be received or transmitted. 
	Normal network nodes are designed to handle packets that are 1518 bytes or less.
	*/
	enc28j60_write_reg(dev, &MAMXFLL, 1518);
	
	/*Configure the Back-to-Back Inter-Packet Gap register, MABBIPG. 
	Most applications will program this register with 15h when Full-Duplex mode is used 
	and 12h when Half-Duplex mode is used.
	*/
	enc28j60_write_reg(dev, &MABBIPG, 0x15);
	
	//Program the local MAC address into the MAADR1:MAADR6 registers.
	enc28j60_write_reg(dev, &MAADR1, my_addr->addr1);
	enc28j60_write_reg(dev, &MAADR2, my_addr->addr2);
	enc28j60_write_reg(dev, &MAADR3, my_addr->addr3);
	enc28j60_write_reg(dev, &MAADR4, my_addr->addr4);
	enc28j60_write_reg(dev, &MAADR5, my_addr->addr5);
	enc28j60_write_reg(dev, &MAADR6, my_addr->addr6);
	
	//Manually set PHCON1.PDPXMD register to set the full duplex mode
	status = enc28j60_read_reg(dev, &PHCON1);
	((PHCON1_REG_16*)(&status))->PDPXMD = 1;
	enc28j60_write_reg(dev, &PHCON1, status);
	
	//Set RX on
	status = enc28j60_read_reg(dev, &ECON1);
	((ECON1_REG*)(&status))->RXEN = 1;
	enc28j60_write_reg(dev, &ECON1, status);
	
	#ifdef LOGGING
		uint8_t init_copleted[] = "ECN28J60 init completed\n";
		dev->io.uart_send_msg(init_copleted);
	#endif
}
