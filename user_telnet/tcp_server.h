#include "stm32f4xx_hal.h"
#include "lwip.h"
#include "tcp.h"
#include "string.h"
#include "fatfs.h"

#define TCP_REMOTE_PORT    8881 /* ???? */
#define TCP_LOCAL_PORT     8880 /* ???? */
void tcp_server_init(void);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err);
