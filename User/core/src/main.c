/*
	Программа работы датчика температуры DS18B20 через OneWire и передачи показаний по Ethernet через SPI и ENC28J60
*/
#include "rcc.h"
#include "enc28j60.h"

int main(void){
	
		SetCoreClock();
		enc28j60_Init();
	
	
    RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;//подаём тактирование на порт С
    GPIOC->CRH&=~GPIO_CRH_CNF13;
    GPIOC->CRH|=GPIO_CRH_MODE13_1;
    GPIOC->ODR=0b1111111111111111;
    
    while(1)
    {
        //GPIOC->BSRR|=GPIO_BSRR_BR13;
        GPIOC->ODR=0;
        for(int i=10000000;i>0;i--); 
        GPIOC->ODR=0b1111111111111111;
        //GPIOC->BSRR|=GPIO_BSRR_BS13;
        for(int i=10000000;i>0;i--);
    }
    
      //GPIOC->ODR |= (1<<13);  
}
