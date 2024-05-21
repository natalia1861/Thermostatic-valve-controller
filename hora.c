#include "cmsis_os2.h"                          // CMSIS RTOS header file

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

static osThreadId_t tid_Thhora;

//timer
static osTimerId_t tim_id1;
static void timer_init(void);
static void timer_start(void);

//variables globales
uint8_t g_hora;
uint8_t g_minutos;
uint8_t g_segundos;
 
void ThreadHora (void *argument);                   
 
int Init_Thhora (void) {
  g_hora=0;
  g_minutos=0;
  g_segundos=0;
  
  tid_Thhora = osThreadNew(ThreadHora, NULL, NULL);
  if (tid_Thhora == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadHora (void *argument) {
  timer_init();
  timer_start();
  while (1) {
		//timer periodico funcionando
    osDelay(10000);
  }
}

static void Timer1_Callback (void const *arg) {
	g_segundos++;
  if(g_segundos==60) {
		g_segundos=0;
    g_minutos++;
    if(g_minutos==60)  {
      g_minutos=0;
      g_hora =(g_hora<24)? g_hora++ : 0;
    }
  }
}

static void timer_init(void){
  tim_id1 = osTimerNew((osTimerFunc_t)&Timer1_Callback, osTimerPeriodic, (void*)0, NULL);
}

static void timer_start(void) {
  osTimerStart(tim_id1, 1000);
}

void timer_stop(void) {
	osTimerStop(tim_id1);
}

void timer_restart(void) {
	osTimerStart(tim_id1, 1000);
	osTimerStart(tim_id1, 1000);
}
