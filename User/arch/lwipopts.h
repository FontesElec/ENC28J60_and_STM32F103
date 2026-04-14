#define NO_SYS 1
#define SYS_LIGHTWEIGHT_PROT 0
#define PBUF_POOL_BUFSIZE 256

#define MEM_ALIGNMENT 4

#define LWIP_ARP 1
#define LWIP_ETHERNET 1
#define LWIP_ICMP 1
#define LWIP_IP 1
#define LWIP_TCP           0
#define LWIP_UDP           0

#define LWIP_SOCKET 0
#define LWIP_NETCONN 0

#define LWIP_STATS 0
#define LWIP_NETIF_STATUS_CALLBACK 0
#define LWIP_NETIF_LINK_CALLBACK 0
#define LWIP_AUTOIP 0
#define LWIP_DHCP 0

#define MEM_SIZE               1024   // было 1600
#define PBUF_POOL_SIZE         1      // было 2
#define MEMP_NUM_PBUF          2
#define MEMP_NUM_UDP_PCB       0
#define MEMP_NUM_TCP_PCB       0
#define MEMP_NUM_TCP_SEG       0
#define MEMP_NUM_SYS_TIMEOUT   3