#include "main.h"
#include "sys.h"
#define SPI_CS_GPIO_Port GPIOB
#define SPI_CS_Pin GPIO_PIN_9
void read_W25Q128_ID();
void W25Q128_test();
void Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void Read_W25Q128_data(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead) ;
void Erase_Write_data_Sector(uint32_t Address,uint32_t Write_data_NUM);
void Write_Word(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q128_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite) ;
