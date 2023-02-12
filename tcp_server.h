#include "stm32f4xx_hal.h"
#include "lwip.h"
#include "tcp.h"
#include "string.h"
#include "fatfs.h"
#define USER_FLASH_END_ADDRESS        FLASH_END
/* Define the user application size */
#define USER_FLASH_SIZE   (FLASH_END - APPLICATION_ADDRESS + 1)

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define APPLICATION_ADDRESS   (uint32_t)0x08040000
#define TCP_REMOTE_PORT    8881 /* ???? */
#define TCP_LOCAL_PORT     8880 /* ???? */
void tcp_server_init(void);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err);
