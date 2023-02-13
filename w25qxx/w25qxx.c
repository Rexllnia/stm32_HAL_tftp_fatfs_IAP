#include "w25qxx.h"
#include "spi.h"
#include "usart.h"
#include <stdio.h>
 
uint32_t FLASH_SIZE=16*1024*1024;	//FLASH 大小为16M字节
uint32_t Data_Address = 4090; //测试地址 250（地址在两页之间） 和 4090 （地址在两扇区并且两页之间）
 
//要写的数据
uint8_t Write_data[]="99999945678901234567890";
#define Write_data_SIZE sizeof(Write_data)
 
//要读的数据
uint8_t Read_data[100] = {0};
#define Read_data_SIZE sizeof(Read_data)
 
/* Nicky ******************************************************************* */
//器件使能
void W25Q128_Enable()
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET); // Chip select
}
 
/* Nicky ******************************************************************* */
//器件失能
void W25Q128_Disable()
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET); // Chip disselect
}
 
/* Nicky ******************************************************************* */
//SPI2 发送 1 个字节数据
void spi2_Transmit_one_byte(uint8_t _dataTx)
{
	HAL_SPI_Transmit(&hspi2,(uint8_t*) &_dataTx,1,HAL_MAX_DELAY);
}
 
/* Nicky ******************************************************************* */
//SPI2 接收 1 个字节数据
uint8_t spi2_Receive_one_byte()
{
	uint16_t _dataRx;
	HAL_SPI_Receive(&hspi2,(uint8_t*) &_dataRx, 1, HAL_MAX_DELAY);
	return _dataRx;
}
 
/* Nicky ******************************************************************* */
//W25Q128写使能,将WEL置1 
void W25Q128_Write_Enable()   
{
	W25Q128_Enable();                            //使能器件   
    spi2_Transmit_one_byte(0x06); 
	W25Q128_Disable();                            //取消片选     	      
}
 
/* Nicky ******************************************************************* */
//W25Q128写失能,将WEL置0 
void W25Q128_Write_Disable()   
{
	W25Q128_Enable();                            //使能器件   
    spi2_Transmit_one_byte(0x04); 
	W25Q128_Disable();                            //取消片选     	      
}
 
/* Nicky ******************************************************************* */
//读取寄存器状态
uint8_t W25Q128_ReadSR(void)   
{  
	uint8_t byte=0;   
	W25Q128_Enable();                            //使能器件   
	spi2_Transmit_one_byte(0x05);    //发送读取状态寄存器命令
	byte=spi2_Receive_one_byte();             //读取一个字节
	W25Q128_Disable();                           //取消片选     
	return byte;   
} 
 
/* Nicky ******************************************************************* */
//等待空闲
void W25Q128_Wait_Busy()   
{   
	while((W25Q128_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}
 
/* Nicky ******************************************************************* */
//擦除地址所在的一个扇区
void Erase_one_Sector(uint32_t Address)
{
	W25Q128_Write_Enable();                  //SET WEL 	 
	W25Q128_Wait_Busy(); 		
	W25Q128_Enable();                            //使能器件 
	spi2_Transmit_one_byte(0x20);      //发送扇区擦除指令 
	spi2_Transmit_one_byte((uint8_t)((Address)>>16));  //发送24bit地址    
	spi2_Transmit_one_byte((uint8_t)((Address)>>8));   
	spi2_Transmit_one_byte((uint8_t)Address);  
	W25Q128_Disable();                            //取消片选     	      
	W25Q128_Wait_Busy(); 				   //等待擦除完成
}
 
 
/* Nicky ******************************************************************* */
//擦除地址所在的扇区
void Erase_Write_data_Sector(uint32_t Address,uint32_t Write_data_NUM)   
{
	//总共4096个扇区
	//计算 写入数据开始的地址 + 要写入数据个数的最后地址 所处的扇区	
	uint16_t Star_Sector,End_Sector,Num_Sector;
	Star_Sector = Address / 4096;						//数据写入开始的扇区
	End_Sector = (Address + Write_data_NUM) / 4096;		//数据写入结束的扇区
	Num_Sector = End_Sector - Star_Sector;  			//数据写入跨几个扇区
 
	//开始擦除扇区
	for(uint16_t i=0;i <= Num_Sector;i++)
	{
		Erase_one_Sector(Address);
		Address += 4095;
	}
 
}
 
/* Nicky ******************************************************************* */
//擦除整个芯片 等待时间超长... 10-20S
void Erase_W25Q128_Chip(void)   
{                                   
    W25Q128_Write_Enable();                  //SET WEL 
    W25Q128_Wait_Busy();   
  	W25Q128_Enable();                            //使能器件   
    spi2_Transmit_one_byte(0x60);        //发送片擦除命令  
	W25Q128_Disable();                            //取消片选     	      
	W25Q128_Wait_Busy();   				   //等待芯片擦除结束
} 
 
/* Nicky ******************************************************************* */
//读取W25Q128数据
void Read_W25Q128_data(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i=0;   										    
	W25Q128_Enable();                     //使能器件   
    spi2_Transmit_one_byte(0x03);         //发送读取命令   
    spi2_Transmit_one_byte((uint8_t)((ReadAddr)>>16));  //发送24bit地址    
    spi2_Transmit_one_byte((uint8_t)((ReadAddr)>>8));   
    spi2_Transmit_one_byte((uint8_t)ReadAddr);   
    for(;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=spi2_Receive_one_byte();   //循环读数  
    }
	W25Q128_Disable(); 				    	      
}
 
/* Nicky ******************************************************************* */
//写字，一次最多一页
void Write_Word(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
 	uint16_t i; 
 
	W25Q128_Write_Enable();                  //SET WEL
	W25Q128_Enable();                            //使能器件
	spi2_Transmit_one_byte(0x02);
    spi2_Transmit_one_byte((uint8_t)((WriteAddr) >> 16)); //写入的目标地址   
    spi2_Transmit_one_byte((uint8_t)((WriteAddr) >> 8));   
    spi2_Transmit_one_byte((uint8_t)WriteAddr);   
    for (i = 0; i < NumByteToWrite; i++)
		spi2_Transmit_one_byte(pBuffer[i]);//循环写入字节数据  
	W25Q128_Disable();
	W25Q128_Wait_Busy();		//写完之后需要等待芯片操作完。
}
 
/* Nicky ******************************************************************* */
//定位到页
void Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{
	uint16_t Word_remain;
	Word_remain=256-WriteAddr%256; 	//定位页剩余的字数	
	
	if(NumByteToWrite <= Word_remain)
		Word_remain=NumByteToWrite;		//定位页能一次写完
	while(1)
	{
		Write_Word(pBuffer,WriteAddr,Word_remain);	
		if(NumByteToWrite==Word_remain)
		{
			break;	//判断写完就 break
		}	
	 	else //没写完，翻页了
		{
			pBuffer += Word_remain;		//直针后移当页已写字数
			WriteAddr += Word_remain;	
			NumByteToWrite -= Word_remain;	//减去已经写入了的字数
			if(NumByteToWrite>256)
				Word_remain=256; 		//一次可以写入256个字
			else 
				Word_remain=NumByteToWrite; 	//不够256个字了
		}
	}	    
}
u8 W25QXX_BUFFER[4096];
void W25Q128_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
  W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		//printf("secpos = %d\r\n",secpos);
		Read_W25Q128_data(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			Erase_one_Sector(secpos);		//擦除这个扇区
			for(i=0;i<secremain;i++)	   		//复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			Write_Page(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  
		}
		else 
		{ 
		  Write_Page(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		}
		if(NumByteToWrite == secremain){ 
			break;//写入结束了
		}
		else //写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		  pBuffer+=secremain;  				//指针偏移
			WriteAddr+=secremain;				//写地址偏移	   
		  NumByteToWrite-=secremain;			//字节数递减
			if(NumByteToWrite > 4096)
			{
				secremain=4096;//下一个扇区还是写不完
			}
			else 
			{
				secremain=NumByteToWrite;		//下一个扇区可以写完了
			}
		}	 
	}	
}
// void Write_Sector(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite) 
// {
//	 
// }
/* Nicky ******************************************************************* */
// 读取 ID 测试 OK 0xEF 0X17
void read_W25Q128_ID()
{
	uint8_t _RxData[2]={0x00};
	W25Q128_Enable();
	
	//发送指令
    spi2_Transmit_one_byte(0x90);
	spi2_Transmit_one_byte(0x00);
	spi2_Transmit_one_byte(0x00);
	spi2_Transmit_one_byte(0x00);
	
	//接收数据
	_RxData[0] = spi2_Receive_one_byte();
	_RxData[1] = spi2_Receive_one_byte();
	
    W25Q128_Disable();
    
//	printf("%s\r\n",_RxData);	//串口打印 ID
}
 
/* Nicky ******************************************************************* */
//测试程序
void W25Q128_test()
{
	//读数据，看原始存在的数据
//	Read_W25Q128_data(Read_data,Data_Address,Read_data_SIZE);	
//	for(uint8_t i=0;i<Write_data_SIZE;i++)
//			printf("%c",Read_data[i]);
//		printf("\r\n");	
//	
//	//擦除需要写数据所在的扇区
 	Erase_Write_data_Sector(0,4096);
	Read_W25Q128_data(Read_data,0,4096);
	for(uint8_t i=0;i<Write_data_SIZE;i++)
			printf("%c",Read_data[i]);
		printf("\r\n");
			
	//写数据
   W25Q128_Write(Write_data,0,4096);
	Read_W25Q128_data(Read_data,0,4096); 
		
	//串口打印数据
	for(uint8_t i=0;i<Write_data_SIZE;i++)
		printf("%c",Read_data[i]);
	printf("\r\n");
}