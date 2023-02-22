/**
  *****************************************************************************
  * @file    tcp_server.c
  * @author  Zorb
  * @version V1.0.0
  * @date    2018-09-04
  * @brief   tcp服务端的实现
  *****************************************************************************
  * @history
  *
  * 1. Date:2018-09-04
  *    Author:Zorb
  *    Modification:建立文件
  *
  *****************************************************************************
  */


#include "tcp_server.h"
#include "path_seq.h"
extern FATFS fs;
extern FIL file;
extern uint8_t res;
extern UINT Br,Bw;

extern uint8_t command;
DIR dp;
FILINFO fno;
struct sequeue dir_seq;
//struct display_handle
//{
//	void *pcb;

//};
void display(void *handle ,char *data)
{
	
		tcp_write(handle,data,strlen(data),1);
		tcp_write(handle,"/",1,1);
}
/******************************************************************************
 * 描述  : 接收回调函数
 * 参数  : -
 * 返回  : -
******************************************************************************/
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb,
                             struct pbuf *p, err_t err)
{
    uint32_t i;

//		struct display_handle *disp_handle;
//		disp_handle->pcb=tpcb;
    /* 数据回传 */
//    tcp_write(tpcb, p->payload, p->len, 1);

    if (p != NULL)
    {
			if(strstr(p->payload,"sdfuse")!=NULL)
			{
					command=1;
					tcp_write(tpcb, p->payload, p->len, 1);
			}
			if(strstr(p->payload,"boot")!=NULL)
			{
					command=2;
					tcp_write(tpcb, p->payload, p->len, 1);
			}
			if(strstr(p->payload,"mkdir ")!=NULL)
			{
				char buf[1000]="0:";
				char str[50] = { 0 };
				char *str1;
				strncpy(str,p->payload,p->len);
				strtok(str,"dir ");
				str1=strtok(NULL,"dir ");
				printf("%s",str1);
				merge_dir(dir_seq,buf);
				sprintf(buf,"%s/%s",buf,str1);
				printf("%s",buf);
				f_mkdir (buf);
			}
			if(strstr(p->payload,"cd ")!=NULL)
			{
				char str[50] = { 0 };
				char *str1;
				strncpy(str,p->payload,p->len);
				
				strtok(str,"d ");
				str1=strtok(NULL,"d ");
				str1=strtok(str1,"\r\n");
				printf("%s",str1);
				if(strstr(str1,"..")!=NULL)
				{
					dir_seq=rear_pop(dir_seq);
				}
				else 
				{
					char *strp=str1;
					while(*strp!='\0')
					{
						if((*strp>='a')&&(*strp<='z'))
						{
							*strp+='A'-'a';
						}
						strp++;
					}
					char buf[1000]="0:";
					merge_dir(dir_seq,buf);
					f_opendir(&dp,buf);
					do{
							f_readdir (&dp, &fno);
							printf("%s",fno.fname);
							if(strcmp(str1,fno.fname)==0)
							{
								
								dir_seq=push(dir_seq,str1);
								break;
							}
					}while(fno.fname[0]!=0);
					f_closedir(&dp);
					
				}
			}
			if(strstr(p->payload,"ls")!=NULL)
			{
					char buf[1000]="0:";
					merge_dir(dir_seq,buf);
					f_opendir(&dp,buf);
					
					do{
							f_readdir (&dp, &fno);
							tcp_write(tpcb, fno.fname,strlen(fno.fname) , 1);
							tcp_write(tpcb, "  ",strlen("  ") , 1);
					}while(fno.fname[0]!=0);
					tcp_write(tpcb, fno.fname,strlen(fno.fname) , 1);
					f_closedir(&dp);
					
					tcp_write(tpcb, "\r\n",strlen("\r\n") , 1);
			}
			Sequeue_display_from_rear(tpcb,dir_seq);
			tcp_write(tpcb, "#",strlen("#") , 1);
        /* 打印接收到的数据 */
        printf("get msg from %d:%d:%d:%d port:%d:\r\n",
            *((uint8_t *)&tpcb->remote_ip.addr),
            *((uint8_t *)&tpcb->remote_ip.addr + 1),
            *((uint8_t *)&tpcb->remote_ip.addr + 2),
            *((uint8_t *)&tpcb->remote_ip.addr + 3),
            tpcb->remote_port);

        tcp_recved(tpcb, p->tot_len);

        /* 释放缓冲区数据 */
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        printf("tcp client closed\r\n");

        tcp_recved(tpcb, p->tot_len);

        return tcp_close(tpcb);
    }

    return ERR_OK;
}

/******************************************************************************
 * 描述  : 客户端接入回调函数
 * 参数  : -
 * 返回  : -
******************************************************************************/
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
		dir_seq=init(dir_seq);//初始化路径
		dir_seq.display=display;
	  dir_seq=push(dir_seq,"0:");
		
    printf("tcp client connected\r\n");

    printf("ip %d:%d:%d:%d port:%d\r\n",
        *((uint8_t *)&newpcb->remote_ip.addr),
        *((uint8_t *)&newpcb->remote_ip.addr + 1),
        *((uint8_t *)&newpcb->remote_ip.addr + 2),
        *((uint8_t *)&newpcb->remote_ip.addr + 3),
        newpcb->remote_port);
		
  tcp_write(newpcb, "\r\n================== Main Menu ============================\r\n\n", strlen("\r\n================== Main Menu ============================\r\n\n"), 1);
	tcp_write(newpcb, "  Download Image To the Internal Flash ------------- sdfuse\r\n\n", strlen("  Download Image To the Internal Flash ------------- sdfuse\r\n\n"), 1);
	tcp_write(newpcb, "  Execute The New Program ---------------------------- boot\r\n\n", strlen("  Execute The New Program ---------------------------- boot\r\n\n"), 1);
	tcp_write(newpcb, "  TFTP port 69 -----------------------------------------\r\n\n", strlen("  TFTP port 69 -----------------------------------------\r\n\n"), 1);
	Sequeue_display_from_rear(newpcb,dir_seq);
	tcp_write(newpcb, "#",strlen("#") , 1);
    /* 注册接收回调函数 */
    tcp_recv(newpcb, tcp_server_recv);

    return ERR_OK;
}

/******************************************************************************
 * 描述  : 创建tcp服务器
 * 参数  : 无
 * 返回  : 无
******************************************************************************/
void tcp_server_init(void)
{
    struct tcp_pcb *tpcb;

    /* 创建tcp控制块 */
    tpcb = tcp_new();

    if (tpcb != NULL)
    {
        err_t err;

        /* 绑定端口接收，接收对象为所有ip地址 */
        err = tcp_bind(tpcb, IP_ADDR_ANY, TCP_LOCAL_PORT);

        if (err == ERR_OK)
        {
            /* 监听 */
            tpcb = tcp_listen(tpcb);

            /* 注册接入回调函数 */
            tcp_accept(tpcb, tcp_server_accept);

            printf("tcp server listening\r\n");
        }
        else
        {
            memp_free(MEMP_TCP_PCB, tpcb);

            printf("can not bind pcb\r\n");
        }

    }
}

/******************************** END OF FILE ********************************/