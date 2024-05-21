#ifndef __THTEMP_H
#define __THTEMP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"

/* Exported types ------------------------------------------------------------*/
typedef struct  {
	  float temp;
} MSGQUEUE_TEMP_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
osMessageQueueId_t Init_ThTemp(void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
