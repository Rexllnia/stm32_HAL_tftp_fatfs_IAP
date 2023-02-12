/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "lwip.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "sys.h"
#include "w25qxx.h"
#include "tcp_server.h"
#include "udp_server.h"
#include "lwip/apps/tftp_server.h"
#include "lwip/pbuf.h"
//#include "lwip/apps/tftp_opts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define debug
BYTE work[4096];
FATFS fs;
FIL file;
uint8_t res=0;
UINT Br,Bw;
char path[4]="0:";
char success[]="1234qqqq!\r\n";
char error[]="error!\r\n";
char mount[]="mount Ok! \r\n";
uint8_t command=0;
DIR dp;
FILINFO fno;
#define TRUE 1
#define FALSE 0
static int file_size=0,pos=0;
uint32_t read_buf[512];
static uint32_t LastPGAddress = APPLICATION_ADDRESS;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void vprint(const char *fmt, va_list argp) {
	char string[200];
	if (0 < vsprintf(string, fmt, argp)) // build string
			{
		HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string),100); // send message via UART
		
	}
}
int fputc(int ch, FILE *f)
{
    while((USART1->SR & 0X40) == 0); 

    USART1->DR = (u8) ch;
    return ch;
}
void my_printf(const char *fmt, ...) // custom printf() function
{
	va_list argp;
	va_start(argp, fmt);
	vprint(fmt, argp);
	va_end(argp);
}
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t i = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  for (i = 0; (i < (length / 4)) && (destination <= (USER_FLASH_END_ADDRESS - 4)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(p_source + i)) == HAL_OK)
    {
      /* Check the written value */
      if (*(uint32_t*)destination != *(uint32_t*)(p_source + i))
      {
        /* Flash content doesn't match SRAM content */
        return 0;
      }
      /* Increment FLASH destination address */
      destination += 4;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return 0;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return 0;
}
typedef void (*pIapFun_TypeDef)(void);
void IAP_ExecuteApp ( uint32_t ulAddr_App )
{
	int i = 0;
	pIapFun_TypeDef pJump2App; 

	
	if ( ( ( * ( __IO uint32_t * ) ulAddr_App ) & 0x2FFE0000 ) == 0x20000000 )	 //�??查栈顶地�??是否合法0x20000000是sram的起始地�??,也是程序的栈顶地�??
	{ 
//		HAL_PCD_MspDeInit(&hpcd_USB_OTG_FS);
		HAL_SPI_MspDeInit(&hspi2);	//@2
		HAL_TIM_Base_MspDeInit(&htim1);
		HAL_TIM_Base_MspDeInit(&htim3);
		__HAL_RCC_GPIOB_CLK_DISABLE();
		__HAL_RCC_GPIOG_CLK_DISABLE();
		
		
		 /* 设置�??有时钟到默认状�?�，使用HSI时钟 */
		HAL_RCC_DeInit();		//@3
		
		__set_BASEPRI(0x20);		//@4
		__set_PRIMASK(1);
        __set_FAULTMASK(1);
		
		/* 关闭�??有中断，清除�??有中断挂起标�?? */
		for (i = 0; i < 8; i++)		//@5
		{
			NVIC->ICER[i]=0xFFFFFFFF;
			NVIC->ICPR[i]=0xFFFFFFFF;
		}
		
		SysTick->CTRL = 0;		//@6
		SysTick->LOAD = 0;
		SysTick->VAL = 0;

		__set_BASEPRI(0);		//@7
		__set_PRIMASK(0);
		__set_FAULTMASK(0);
 		
 		//@8
        /*
        1）不使用OS时： 只用到MSP（中断和非中断都使用MSP）；
        2）使用OS时（如UCOSII）： main函数和中断使用MSP�?? 各个Task（线程）使用PSP（即任务栈）�??
        */
        __set_MSP(*(uint32_t*)ulAddr_App);//当带操作系统从APP区跳转到BOOT区的时�?�需要将SP设置为MSP，否则在BOOT区中使用中断将会引发硬件错误�??
        __set_PSP(*(uint32_t*)ulAddr_App);
        __set_CONTROL(0);  /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
        __ISB();//指令同步隔离。最严格：它会清洗流水线，以保证�??有它前面的指令都执行完毕之后，才执行它后面的指令�??
		//@9
		pJump2App = ( pIapFun_TypeDef ) * ( __IO uint32_t * ) ( ulAddr_App + 4 );	//用户代码区第二个字为程序�??始地�??(复位地址)		
		pJump2App ();								                                    	//跳转到APP.
	}
}	
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  if (htim->Instance==TIM1)
	{
		MX_LWIP_Process();
	}
	if (htim->Instance==TIM3)
	{

		if(command==2)
		{
			printf("tim3\r\n");
			IAP_ExecuteApp (APPLICATION_ADDRESS);
		}
		else if(command==1)
		{
						HAL_FLASH_Unlock();	
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_WRPERR);
			FLASH_EraseInitTypeDef pEraseInit;

      
      pEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;
      pEraseInit.Sector=FLASH_SECTOR_6;
      pEraseInit.NbSectors=4;
			uint32_t SectorError;
			HAL_FLASHEx_Erase(&pEraseInit,&SectorError);
      HAL_FLASH_Lock();
			f_open(&file,"test_IAP.bin",FA_READ);
			__IO uint32_t read_size = 0x00, tmp_read_size = 0x00;
				uint32_t read_flag = TRUE;

				/* Erase address init */
				LastPGAddress = APPLICATION_ADDRESS;

				/* While file still contain data */
				while (read_flag == TRUE) {

					/* Read maximum "BUFFERSIZE" Kbyte from the selected file  */
					f_read(&file, read_buf, 512,(UINT*) &read_size);

					/* Temp variable */
					tmp_read_size = read_size;
					/* The read data < "BUFFERSIZE" Kbyte */
					if (tmp_read_size < 512) {
						read_flag = FALSE;
					}
					/* Program flash memory */
					FLASH_If_Write(LastPGAddress, (uint32_t*) read_buf, read_size);
					/* Update last programmed address value */
					LastPGAddress = LastPGAddress + tmp_read_size;
				}
			f_close(&file);
				
		}
		command=0;

	}
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_LWIP_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	#ifdef debug
	my_printf("START\r\n");
	read_W25Q128_ID();
	W25Q128_test();
	#endif
//	res=f_mkfs("0:",FM_ANY,4096,work,sizeof(work));//format
//	printf("format result : %d\r\n",res);
	res=f_mount(&fs,"0:",0);
	f_opendir(&dp,"0:");
	f_readdir (&dp, &fno);
	printf("%s\r\n",fno.fname);
	f_closedir(&dp);
//						HAL_FLASH_Unlock();	
//      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_WRPERR);
//			FLASH_EraseInitTypeDef pEraseInit;

//      
//      pEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;
//      pEraseInit.Sector=FLASH_SECTOR_6;
//      pEraseInit.NbSectors=4;
//			uint32_t SectorError;
//			HAL_FLASHEx_Erase(&pEraseInit,&SectorError);
//      HAL_FLASH_Lock();

	#ifdef debug
	if(res!=FR_OK){
		HAL_UART_Transmit(&huart1,(uint8_t *) &error,sizeof(error),100);
	}else{
		HAL_UART_Transmit(&huart1,(uint8_t *) &mount,sizeof(mount),100);
	}


	res = f_open(&file, "aaa.txt", FA_OPEN_ALWAYS|FA_WRITE);
	f_lseek(&file, f_size(&file));
	
	f_write(&file,success,strlen(success),&Bw);
	f_close(&file);
	char rbuf[20]="";
	res = f_open(&file, "aaa.txt", FA_OPEN_ALWAYS|FA_READ);

	f_read(&file,rbuf,19,&Br);
	my_printf("%s",rbuf);
	f_close(&file);
	udp_server_init();
	#endif
	
	tcp_server_init();
	tftp_init();
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
