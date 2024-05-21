#ifndef __THHORA_H
#define __THHORA_H


#include "cmsis_os2.h"


extern uint8_t g_hora;
extern uint8_t g_minutos;
extern uint8_t g_segundos;

int Init_Thhora (void);
void timer_stop(void);
void timer_restart(void);

#endif
