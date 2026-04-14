/*
	Программа работы датчика температуры DS18B20 через OneWire и передачи показаний по Ethernet через SPI и ENC28J60
*/
#include "rcc.h"
#include "enc28j60.h"
#include "init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include "lwip/init.h"

volatile uint32_t sys_time = 0;
//extern volatile uint32_t sys_time;

void SysTick_Handler(void){
    sys_time++;
}

u32_t sys_now(void){
    return sys_time;
}


static struct netif netif;

struct pbuf *low_level_input(struct netif *netif, uint8_t* buf, uint16_t buf_size){
	//uint8_t buf[1520];
  uint16_t len = enc28j60_receive_packet(buf, buf_size);
  if (len == 0){
		return NULL;
	}
  struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  if(!p){
		return NULL;
	}
	pbuf_take(p, buf, len);
  return p;
}

err_t low_level_output(struct netif *netif, struct pbuf *p){
	enc28j60_prepare_to_tx();
	uint8_t control = 0x00;
	enc28j60_write_to_tx(&control, 1);
	for (struct pbuf *q = p; q != NULL; q = q->next){
		enc28j60_write_to_tx((uint8_t *)q->payload, q->len);
	}
	enc28j60_ready_to_send();
	return ERR_OK;
}	
	
err_t ethernetif_init(struct netif *netif)
{
    netif->name[0] = 'e';
    netif->name[1] = 'n';

    netif->output     = etharp_output;
    netif->linkoutput = low_level_output;

    netif->hwaddr_len = ETH_HWADDR_LEN;

    netif->hwaddr[0] = 0x02;
    netif->hwaddr[1] = 0x00;
    netif->hwaddr[2] = 0x00;
    netif->hwaddr[3] = 0x00;
    netif->hwaddr[4] = 0x00;
    netif->hwaddr[5] = 0x01;

    netif->mtu = 1500;

   netif->flags =  NETIF_FLAG_BROADCAST |
									 NETIF_FLAG_ETHARP |
									 NETIF_FLAG_LINK_UP |
									 NETIF_FLAG_ETHERNET;
									 
    return ERR_OK;
}

int main(void){
	
		//CoreClock settings
		SetCoreClock();
	
		//SysTick settings
		SysTick_Config(SystemCoreClock / 1000);
	
		//PORT settings
    RCC->APB2ENR|=RCC_APB2ENR_IOPCEN;//подаём тактирование на порт С
    GPIOC->CRH&=~GPIO_CRH_CNF13;
    GPIOC->CRH|=GPIO_CRH_MODE13_1;
    GPIOC->ODR=0b1111111111111111;

	
		//ENC part
	  static uint8_t eth_buffer[1520];
		static MAC_addr_t my_MAC_addr = {02, 00, 00, 00, 00, 01};
		static eth_frame_t eth_frame;
		static ETH_rx_header_t rx_header;
		
		enc28j60_Init(&my_MAC_addr, eth_buffer, &rx_header, &eth_frame);
		
		//LWIP  part
		ip4_addr_t ipaddr, netmask, gw;

		IP4_ADDR(&ipaddr, 169,254,25,63);
		//IP4_ADDR(&ipaddr, 192,168,1,100);
		IP4_ADDR(&netmask, 255,255,255,0);
		IP4_ADDR(&gw, 192,168,1,1);
		
		lwip_init();
		
    netif_add(&netif, &ipaddr, &netmask, &gw,
              NULL, ethernetif_init, ethernet_input);

    netif_set_default(&netif);
    netif_set_up(&netif);
		
    while (1){
			GPIOC->ODR ^= (1 << 13);
			for(volatile int i = 0; i < 100000; i++);
			struct pbuf *p = low_level_input(&netif, eth_buffer, sizeof(eth_buffer));
			if (p != NULL){
				if (netif.input(p, &netif) != ERR_OK){
					pbuf_free(p);
				}
			}
			sys_check_timeouts();
			GPIOC->ODR ^= (1 << 13);
			for(volatile int i = 0; i < 100000; i++);
		}
}
