#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "principal.h"
#include "joystick.h"
#include "hora.h"
#include "temp.h"
#include "lcd.h"
#include "pwm.h"
#include "pot.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Principal
 *---------------------------------------------------------------------------*/
 
#define N_MEDIDAS_MAX 10
#define TEMPERATURA_REF 25
 
 //thread
osThreadId_t tid_Thread_principal;                        // thread id
void Thread_principal (void *argument);                   // thread function

//Control States
typedef enum {REPOSO, ACTIVO, TEST, PROG_DEP} control_state_t;
static control_state_t control_states;

//tipos de variables
typedef struct {
	uint16_t hora;
  uint16_t minutos;
  uint16_t segundos;
	float temp_med;
	float temp_ref;
	uint8_t duty;
} MSGBUFFER_t;

//buffer circular
static MSGBUFFER_t buffer[N_MEDIDAS_MAX];
static uint8_t puntero_buff;

//COLAS
static osMessageQueueId_t Joystick_queue;
static osMessageQueueId_t LCD_Queue;
static osMessageQueueId_t Temperature_Queue;
static osMessageQueueId_t Pwm_Queue;
static osMessageQueueId_t Pot_Queue;

//MENSAJES
static MSGQUEUE_JOY_t msg_joy;
static MSGQUEUE_TEMP_t msg_temp;
static MSGQUEUE_LCD_t msg_lcd;
static MSGQUEUE_PWM_t msg_pwm;
static MSGQUEUE_POT_t msg_pot;

//extern global
extern uint8_t g_hora;
extern uint8_t g_minutos;
extern uint8_t g_segundos;

//local functions
static void init_ctrl(void);
static void init_leds(void);
static void init_rgb(void);
static void lcd_clean(void);
static int init_modulos(void);
static void configurar_modo(float tempRef, float tempMed);
static void introducir_medidas(void);

//local variables
float temp_ref_configurable;
int duty_cycle;

int Init_Thread_principal (void) {
  tid_Thread_principal = osThreadNew(Thread_principal, NULL, NULL);
  if (tid_Thread_principal == NULL) {
    return(-1);
  }
	return 0;
}
 
void Thread_principal (void *argument) {
	//status, se activa cuando se recibe algun mensaje/flag de algun hilo
	static osStatus_t status_joystick;
  static osStatus_t status_temp;
	static osStatus_t status_pot;
	
  init_modulos();
	init_ctrl();
	
  while (1) {
    status_joystick = osMessageQueueGet(Joystick_queue, &msg_joy, NULL, 0U);
    status_temp = osMessageQueueGet(Temperature_Queue, &msg_temp, NULL, 0U);
		status_pot = osMessageQueueGet(Pot_Queue, &msg_pot, NULL, 0U);
    switch (control_states) {
      case REPOSO:
        sprintf(msg_lcd.mensaje1,"  SBM2022  T:%.1lf$C",msg_temp.temp);
        sprintf(msg_lcd.mensaje2,"      %d%d:%d%d:%d%d      ", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
        osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
        if (status_joystick==osOK && msg_joy.largo_corto == true && msg_joy.joy==16) {
					lcd_clean();
					sprintf(msg_lcd.mensaje1,"  ACT---%d%d:%d%d:%d%d---", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
					sprintf(msg_lcd.mensaje2,"Tm:%.1lf$-Tr:%d$-D:%u%%", msg_temp.temp, TEMPERATURA_REF, duty_cycle);
					osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
					configurar_modo(TEMPERATURA_REF, msg_temp.temp);
					introducir_medidas();
					control_states = ACTIVO;
        }
      break;
      case ACTIVO:
				sprintf(msg_lcd.mensaje1,"  ACT---%d%d:%d%d:%d%d---", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
				sprintf(msg_lcd.mensaje2,"Tm:%.1lf$-Tr:%d$-D:%d%%", msg_temp.temp, TEMPERATURA_REF, duty_cycle);
				osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
				if (status_temp == osOK) {
					configurar_modo(TEMPERATURA_REF, msg_temp.temp);
					introducir_medidas();
					//osMessageQueuePut(Pwm_Queue, &msg_pwm, 0U, 0U);
				}
				if (status_joystick==osOK && msg_joy.largo_corto == true && msg_joy.joy==16) {
					lcd_clean();
					sprintf(msg_lcd.mensaje1,"TEST---%d%d:%d%d:%d%d---", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
					sprintf(msg_lcd.mensaje1,"Tm:%.1lf$-Tr:%d$-D:%d%%", msg_temp.temp, TEMPERATURA_REF, duty_cycle);
					osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
					configurar_modo(msg_pot.temp_referencia, msg_pot.temp_medida);
					control_states = TEST;
				}
			break;
      case TEST:
				sprintf(msg_lcd.mensaje1,"TEST---%d%d:%d%d:%d%d---", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
				sprintf(msg_lcd.mensaje1,"Tm:%.1lf$-Tr:%d$-D:%d%%", msg_temp.temp, TEMPERATURA_REF, duty_cycle);
				osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
				if (status_pot == osOK) {
					configurar_modo(msg_pot.temp_referencia, msg_pot.temp_medida);
					osMessageQueuePut(Pwm_Queue, &msg_pwm, 0U, 0U);
				}
				if (status_joystick==osOK && msg_joy.largo_corto == true && msg_joy.joy==16) {
					lcd_clean();
					sprintf(msg_lcd.mensaje1,"     ---P&&D---     ");
					sprintf(msg_lcd.mensaje1,"H:%d%d:%d%d:%d%d---Tr:%.1lf$", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10,
					temp_ref_configurable);
					osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
					control_states = PROG_DEP;
				}
      break;
      case PROG_DEP:
				
        break;
    }
  }
}

static void init_ctrl(void) {
	//init variables
	duty_cycle = 0;
	puntero_buff = 0;
	temp_ref_configurable = 25;
	init_leds();
	init_rgb();
	
	 //init state
  sprintf(msg_lcd.mensaje1,"  SBM2022  T:%.1lf$C",msg_temp.temp);
  sprintf(msg_lcd.mensaje2,"      %d%d:%d%d:%d%d      ", g_hora/10, g_hora%10, g_minutos/10, g_minutos%10, g_segundos/10, g_segundos%10);
  osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, 0U);
	control_states = REPOSO;
}

static int init_modulos(void) {
  Init_Thhora();
  Joystick_queue = Init_Thjoy();
  Temperature_Queue = Init_ThTemp();
  LCD_Queue = Init_Thlcd();
	Pwm_Queue = Init_ThPwm();
  Pot_Queue = Init_ThPot();
  if (Joystick_queue == NULL | Temperature_Queue == NULL) {
    return -1;
  }
  return 0;
}

static void init_leds(void) {
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

static void init_rgb(void) {
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

//funcion que calcula el ciclo de trabajo necesario durante el MODO ACTIVO y ademas proporciona RGB necesario
static void configurar_modo(float tempRef, float tempMed) {
	static float x;
	x = tempRef - tempMed;
	if (x > 5) {
		duty_cycle = 100;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	} else if (x > 0) {
		duty_cycle = 80;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
	} else if (x > -5) {
		duty_cycle = 40;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	} else {
		duty_cycle = 0;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
	}
	msg_pwm.duty = duty_cycle;
}

static void introducir_medidas(void) {
	static MSGBUFFER_t medidas;
	medidas.hora = g_hora;
	medidas.minutos = g_minutos;
	medidas.segundos = g_segundos;
	medidas.temp_med = msg_temp.temp;
	medidas.temp_ref = TEMPERATURA_REF;
	medidas.duty = duty_cycle;
	buffer[puntero_buff] = medidas;
	if (puntero_buff > N_MEDIDAS_MAX) {
		puntero_buff = 0;
	} else {
		puntero_buff++;
	}
}

static void lcd_clean(void) {
  msg_lcd.clean = true;
  osMessageQueuePut(LCD_Queue, &msg_lcd, 0U, osWaitForever);
  msg_lcd.clean = false;
}

