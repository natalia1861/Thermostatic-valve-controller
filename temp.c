#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "temp.h"

#define QUEUE_MAX 10
#define FLAG_TEMP 0x0001

#define TEMP_I2C_ADDR       0x48
#define REG_TEMP            0x00

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

//I2C DRIVER

extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
 
static volatile uint32_t I2C_Event;
static float calc;

//internal functions
static void init_LM75B(void);

//internal variables
static MSGQUEUE_TEMP_t temperature;
static float temp_aux;
static uint8_t cmd;
static uint8_t buffer_temp[5];

//timer pulsacion larga
static osTimerId_t id_timer_temp;
static void timer_temp_callback (void *arg);
static void init_timer_temp (void);
static void timer_temp_start (void);

//thread stuff
static osThreadId_t tid_Thread_temp;                        // thread id
static void Thread_temp (void *argument);                   // thread function
 
//queue stuff
static osMessageQueueId_t queue;

osMessageQueueId_t Init_ThTemp (void) {
	queue = osMessageQueueNew(QUEUE_MAX, sizeof(MSGQUEUE_TEMP_t), NULL);
  tid_Thread_temp = osThreadNew(Thread_temp, NULL, NULL);
  if (tid_Thread_temp == NULL) {
    return(NULL);
  }
 
  return(queue);
}
 
void Thread_temp (void *argument) {
  init_LM75B();
  init_timer_temp();
  timer_temp_start();
  //first measure
    I2Cdrv->MasterTransmit (TEMP_I2C_ADDR, &cmd, 2, true);
    while (I2Cdrv->GetStatus().busy){//Bus busy, espera hasta ser liberado
    }
    I2Cdrv->MasterReceive(TEMP_I2C_ADDR,buffer_temp ,2,false);//Lee temp
    while (I2Cdrv->GetStatus().busy){//espera liberacion del bus
    }
    temp_aux=((buffer_temp[0]<<8)|(buffer_temp[1]))>>5;
    calc=temp_aux*(0.125);
    temperature.temp = calc;
    osMessageQueuePut(queue, &temperature, 0U, osWaitForever);
  while (1) {
    osThreadFlagsWait(FLAG_TEMP,osFlagsWaitAny,osWaitForever);//wait flag 1s
      
    I2Cdrv->MasterTransmit (TEMP_I2C_ADDR, &cmd, 2, true);
    while (I2Cdrv->GetStatus().busy){//Bus busy, espera hasta ser liberado
    }
    I2Cdrv->MasterReceive(TEMP_I2C_ADDR,buffer_temp ,2,false);//Lee temp
    while (I2Cdrv->GetStatus().busy){//espera liberacion del bus
    }
    temp_aux=((buffer_temp[0]<<8)|(buffer_temp[1]))>>5;
    calc=temp_aux*(0.125);
    temperature.temp = calc;
    osMessageQueuePut(queue, &temperature, NULL, osWaitForever);
  }
}

//timer temperatura
static void init_timer_temp(void) {
	id_timer_temp = osTimerNew(timer_temp_callback, osTimerPeriodic, NULL, NULL);
	if (id_timer_temp != NULL) {
		//funciona bien
	}
}
static void timer_temp_start (void) {
	osTimerStart(id_timer_temp, 1000);
}
static void timer_temp_callback (void *arg) {
	osThreadFlagsSet(tid_Thread_temp, FLAG_TEMP);
}

static void init_LM75B(void) {
  I2Cdrv->Initialize (NULL);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);
  cmd=REG_TEMP;
}
