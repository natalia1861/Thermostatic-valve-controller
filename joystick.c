#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "joystick.h"

#define FLAG_FIN_REB 0X0001
#define FLAG_PULSACION 0X0010
#define FLAG_PL 0X0100
#define QUEUE_MAX 10

typedef struct {
  uint16_t pin;
  GPIO_TypeDef * port;
} pin_puerto;

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Thjoy
 *---------------------------------------------------------------------------*/
//internal functions
static void Init_Joystick(void);
static void init_boton(void);

//timer rebotes
static osTimerId_t id_timer_rebotes;
static void timer_rebotes_callback (void *arg);
static void init_timer_rebotes(void);
static void timer_rebotes_start (void);

//timer pulsacion larga
static osTimerId_t id_timer_pl;
static void timer_pl_callback (void *arg);
static void init_timer_pl (void);
static void timer_pl_start (void);

//thread
static osThreadId_t tid_Thread_joystick;                        // thread id
static void Thread_Joystick (void *argument);                   // thread function
 
 //internal variables
static pin_puerto pin_pulsado;
static MSGQUEUE_JOY_t msg_joy;
static osMessageQueueId_t queue;
static uint32_t flag_detectado;

osMessageQueueId_t Init_Thjoy (void) {
	queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_JOY_t), NULL);
  tid_Thread_joystick = osThreadNew(Thread_Joystick, NULL, NULL);
  if (tid_Thread_joystick == NULL) {
    return(NULL);
  }
 
  return(queue);
}
 
void Thread_Joystick (void *argument) {
  Init_Joystick();
	init_boton();
	init_timer_rebotes();
	init_timer_pl();
  while (1) {
		flag_detectado = osThreadFlagsWait(FLAG_PULSACION | FLAG_FIN_REB | FLAG_PL, osFlagsWaitAny, osWaitForever);
		switch (flag_detectado) {
			case FLAG_PULSACION:
				timer_rebotes_start();
				timer_pl_start();
			break;
			case FLAG_FIN_REB:
				if (HAL_GPIO_ReadPin(pin_pulsado.port, pin_pulsado.pin) == GPIO_PIN_RESET) {
					switch (pin_pulsado.pin) {
						case GPIO_PIN_10: //UP
							msg_joy.joy = 1;
						break;
						case GPIO_PIN_11: //RIGHT
							msg_joy.joy = 2;
						break;
						case GPIO_PIN_12: //DOWN
							msg_joy.joy = 4;
						break;
						case GPIO_PIN_13: //BLUE BUTTON
							msg_joy.joy = 32;
						break;
						case GPIO_PIN_14: //LEFT
							msg_joy.joy = 8;
						break;
						case GPIO_PIN_15: //CENTER
							msg_joy.joy = 16;
						break;
					}
					osMessageQueuePut(queue, &msg_joy, NULL, osWaitForever);
					msg_joy.largo_corto = 0;
				} else {
					timer_rebotes_start();
				}
			break;
			case FLAG_PL:
				msg_joy.largo_corto = 1;
			break;	
		}
	}
}

//timer rebotes
void init_timer_rebotes(void) {
	id_timer_rebotes = osTimerNew(timer_rebotes_callback, osTimerOnce, NULL, NULL);
	if (id_timer_rebotes != NULL) {
		//funciona bien
	}
}
void timer_rebotes_start (void) {
	osTimerStart(id_timer_rebotes, 50);
}
void timer_rebotes_callback (void *arg) {
	osThreadFlagsSet(tid_Thread_joystick, FLAG_FIN_REB);
}

//timer pulsacion larga
void init_timer_pl(void) {
	id_timer_pl = osTimerNew(timer_pl_callback, osTimerOnce, NULL, NULL);
	if (id_timer_pl != NULL) {
		//funciona bien
	}
}
void timer_pl_start (void) {
	osTimerStart(id_timer_pl, 2000);
}
void timer_pl_callback (void *arg) {
	osThreadFlagsSet(tid_Thread_joystick, FLAG_PL);
}

void Init_Joystick (){
  static GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
    //UP 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
  
    //RIGHT 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 

    //DOWN
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct); 
  
    //LEFT
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
    //CENTER
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void init_boton (void) {
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
void EXTI15_10_IRQHandler (void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
 }
    
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	osThreadFlagsSet(tid_Thread_joystick, FLAG_PULSACION);
  switch (GPIO_Pin) {
    case GPIO_PIN_10:
			pin_pulsado.pin = GPIO_PIN_10;
		  pin_pulsado.port = GPIOB;
    break;
		case GPIO_PIN_11:
			pin_pulsado.pin = GPIO_PIN_11;
		  pin_pulsado.port = GPIOB;;
    break;
		case GPIO_PIN_12:
			pin_pulsado.pin = GPIO_PIN_12;
		  pin_pulsado.port = GPIOE;
    break;
		case GPIO_PIN_13:
			pin_pulsado.pin = GPIO_PIN_13;
		  pin_pulsado.port = GPIOC;
    break;
		case GPIO_PIN_14:
			pin_pulsado.pin = GPIO_PIN_14;
		  pin_pulsado.port = GPIOE;
    break;
		case GPIO_PIN_15:
			pin_pulsado.pin = GPIO_PIN_15;
		  pin_pulsado.port = GPIOE;
    break;
		
  }
}
