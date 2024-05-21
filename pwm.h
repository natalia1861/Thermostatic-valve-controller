#ifndef __THPWM_H
#define __THPWM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

/* Exported types ------------------------------------------------------------*/
typedef struct{
  uint8_t duty;
} MSGQUEUE_PWM_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
osMessageQueueId_t Init_ThPwm (void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
