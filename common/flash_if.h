#include "main.h"
#define USER_FLASH_END_ADDRESS        FLASH_END
/* Define the user application size */
#define USER_FLASH_SIZE   (FLASH_END - APPLICATION_ADDRESS + 1)

/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define APPLICATION_ADDRESS   (uint32_t)0x08040000
#define TRUE 1
#define FALSE 0
typedef void (*pIapFun_TypeDef)(void);
void IAP_ExecuteApp ( uint32_t ulAddr_App );
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);