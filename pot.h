#ifndef __THPOT_H
#define __THPOT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "math.h"

/* Exported types ------------------------------------------------------------*/
typedef struct{
  float temp_medida;
	float temp_referencia;
} MSGQUEUE_POT_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifndef __ADC_H
	void ADC1_pins_F429ZI_config(void);
	int ADC_Init_Single_Conversion(ADC_HandleTypeDef*, ADC_TypeDef  *);
	float ADC_getVoltage(ADC_HandleTypeDef* , uint32_t );
#endif
osMessageQueueId_t Init_ThPot (void);
  /* Exported thread functions,  
  Example: extern void app_main (void *arg); */
#endif /* __MAIN_H */
