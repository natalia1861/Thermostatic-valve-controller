#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "pot.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

#define QUEUE_MAX 10
#define RESOLUTION_12B 4096U
#define VREF 3.3f
#define TEMP_RESOLUTION 0.5f
	
//thread stuff
static osThreadId_t tid_Thread_pot;                        // thread id
static void Thread_pot (void *argument);                   // thread function

//queue
static osMessageQueueId_t pot_queue;
static MSGQUEUE_POT_t pot_msg;

osMessageQueueId_t Init_ThPot (void) {
	pot_queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_POT_t), NULL);
  tid_Thread_pot = osThreadNew(Thread_pot, NULL, NULL);
  if (tid_Thread_pot == NULL) {
    return(NULL);
  }
 
  return(pot_queue);
}
 
static void Thread_pot (void *argument) {
	float value_pot1;
  float value_pot2;
	float temp_medida;
	float temp_referencia;
	
 	ADC_HandleTypeDef adchandle; //handler definition
	ADC1_pins_F429ZI_config(); //specific PINS configuration

	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
	value_pot1 = ADC_getVoltage(&adchandle , 10 ); //get values from channel 10->ADC123_IN10
	temp_medida = 25*value_pot1/3.3f + 5;
	value_pot2=ADC_getVoltage(&adchandle , 13 );
	temp_referencia = 25*value_pot2/3.3f + 5;
	pot_msg.temp_medida = temp_medida;
	pot_msg.temp_referencia = temp_referencia;
	osMessageQueuePut(pot_queue, &pot_msg, 0U, osWaitForever);
  
  while (1) {
		value_pot1 = ADC_getVoltage(&adchandle , 10); //get values from channel 10->ADC123_IN10
		temp_medida = 25*value_pot1/3.3f + 5;
		value_pot2=ADC_getVoltage(&adchandle , 13);
		temp_referencia = 25*value_pot2/3.3f + 5;
		if (fabs(temp_medida-pot_msg.temp_medida) > TEMP_RESOLUTION | fabs(temp_referencia-pot_msg.temp_referencia) > TEMP_RESOLUTION) {
			pot_msg.temp_medida = temp_medida;
			pot_msg.temp_referencia = temp_referencia;
			osMessageQueuePut(pot_queue, &pot_msg, 0U, osWaitForever);
		}
  }
}

void ADC1_pins_F429ZI_config(){
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/*PC0     ------> ADC1_IN10
    PC3     ------> ADC1_IN13
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
/**
  * @brief Initialize the ADC to work with single conversions. 12 bits resolution, software start, 1 conversion
  * @param ADC handle
	* @param ADC instance
  * @retval HAL_StatusTypeDef HAL_ADC_Init
  */
int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance) {
   /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
 hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    return -1;
  }
	return 0;
}

/**
  * @brief Configure a specific channels ang gets the voltage in float type. This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick()
  * @param ADC_HandleTypeDef
	* @param channel number
	* @retval voltage in float (resolution 12 bits and VRFE 3.3
  */
float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel) {
		ADC_ChannelConfTypeDef sConfig = {0};
		HAL_StatusTypeDef status;

		uint32_t raw = 0;
		float voltage = 0;
		 /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
  {
    return -1;
  }
		HAL_ADC_Start(hadc);
		do (
			status = HAL_ADC_PollForConversion(hadc, 0)); //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
		while(status != HAL_OK);
		raw = HAL_ADC_GetValue(hadc);
		voltage = raw*VREF/RESOLUTION_12B; 
		return voltage;
}
