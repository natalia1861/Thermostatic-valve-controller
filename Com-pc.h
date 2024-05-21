#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "Driver_USART.h"

#define RX_BUFFER_SIZE 32
#define TX_BUFFER_SIZE 32
#define MSGQUEUE_OBJECTS 16 
#define RECEIVE_COMPLETE_FLAG 0x0002U
#define SEND_COMPLETE_FLAG 0X0001U

extern osThreadId_t          tid_Thread_COM;                        
extern osMessageQueueId_t    mid_comQueue;



static void init_Usart(void);
void stopUSART(void);
void resumeUSART(void);
void myUSART_callback(uint32_t event);
osMessageQueueId_t Init_Thcom (void);
void convertirBytesRxACaracter(char *array);
void respuesta(void);
void procesarCasosRx(void);
void respuesta(void);
void ThreadCom (void *argument);
static void init_Usart(void);
void procesarDatos(void);
void cleanRxBuff(void);
