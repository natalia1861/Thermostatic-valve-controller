#ifndef __THJOY_H
#define __THJOY_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "stdbool.h"
/* Exported types ------------------------------------------------------------*/
typedef struct  {
	  uint8_t joy;
    bool largo_corto;
} MSGQUEUE_JOY_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
osMessageQueueId_t Init_Thjoy(void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
