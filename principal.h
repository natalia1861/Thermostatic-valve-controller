#ifndef __THPRIN_H
#define __THPRIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "stdbool.h"

/* Exported types ------------------------------------------------------------*/
typedef struct  {
	  uint8_t joy;
    bool largo_corto;
} joy_5bits_rec;
/* Exported constants --------------------------------------------------------*/
#define QUEUE_MAX 10
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int Init_Thread_principal(void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
