#include "flash_if.h"

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
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi2;
extern ETH_HandleTypeDef heth;
void IAP_ExecuteApp ( uint32_t ulAddr_App )
{
	int i = 0;
	pIapFun_TypeDef pJump2App; 

	
	if ( ( ( * ( __IO uint32_t * ) ulAddr_App ) & 0x2FFE0000 ) == 0x20000000 )	 //�??查栈顶地�??是否合法0x20000000是sram的起始地�??,也是程序的栈顶地�??
	{ 
//		HAL_PCD_MspDeInit(&hpcd_USB_OTG_FS);
		HAL_SPI_MspDeInit(&hspi2);	//@2
		HAL_ETH_MspDeInit(&heth);
//		HAL_TIM_Base_MspDeInit(&htim1);
//		HAL_TIM_Base_MspDeInit(&htim3);
		HAL_UART_MspDeInit(&huart1);
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
		__disable_irq();
 		
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