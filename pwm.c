#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "pwm.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
 #define QUEUE_MAX 10
 
 //thread stuff
static osThreadId_t tid_Thread_pwm;                        // thread id
void Thread_pwm (void *argument);                   // thread function

//PWM
TIM_HandleTypeDef htim1;  //estructura tim1 PWM mode
static TIM_OC_InitTypeDef sConfigOC; //config tim1 PWM mode
static GPIO_InitTypeDef GPIO_InitStruct; //estructura config pin salida TIM1

//variables
static uint16_t dutyCycle; //ciclo de trabajo

//internal functions
static void initTim1PWM(void);
static void reinitTIM1PWM (uint8_t duty);
static void initPinPE9(void);

//queue
static osMessageQueueId_t pwm_queue;
static MSGQUEUE_PWM_t msg_reception;

osMessageQueueId_t Init_ThPwm (void) {
  pwm_queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_PWM_t), NULL);
  tid_Thread_pwm = osThreadNew(Thread_pwm, NULL, NULL);
  if (tid_Thread_pwm == NULL) {
    return(NULL);
  }
  return(pwm_queue);
}
 
void Thread_pwm (void *argument) {
  initTim1PWM();
	initPinPE9();
  while (1) {
    osMessageQueueGet(pwm_queue, &msg_reception, NULL, osWaitForever);
		reinitTIM1PWM(msg_reception.duty);
  }
}

static void initTim1PWM(void) { //funcion config tim1 PWM
  __HAL_RCC_TIM1_CLK_ENABLE();
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 4799;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim1);
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
}

static void reinitTIM1PWM (uint8_t duty) {
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	sConfigOC.Pulse = duty;			//quizas es duty-1
	HAL_TIM_PWM_Init(&htim1);
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  dutyCycle = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
}
static void initPinPE9(void){ //Pin salida PE9 TIM1
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}
