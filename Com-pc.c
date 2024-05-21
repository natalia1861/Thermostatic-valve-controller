#include "Com-pc.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static osThreadId_t          tid_Thread_COM; 
static osMessageQueueId_t    mid_comQueue;
static osEventFlagsId_t ThCom_id; 

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

//bariables de gestion de buffer de entra
	uint8_t rxBuffer;
	uint8_t bufferDefinitivo[12];  // Buffer para almacenar los datos entre 01 y FE
  uint8_t indexDefinitivo = 0;
	
	//estructura que se envia
typedef struct {                                // object data type
  uint16_t Id;
	uint16_t hora;
  uint16_t minutos;
  uint16_t segundos;
	float temp;
	char medidas[10];
} MSGQUEUE_OBJ_t;

static MSGQUEUE_OBJ_t msgRx;
osMessageQueueId_t Init_Thcom (void);

	//VARIABLES PARA LA RESPUESTA, se responde, en el caso 3 se piden medidas, limpiamos el bufferDefinitivo[1]
		char medida[32];
    char respPuestaHora[31];
    char respuestareftemp[21];
    char Borrrarmedidas[17];
		uint8_t r = 0;

osMessageQueueId_t Init_Thcom(void) 
	{
	tid_Thread_COM = osThreadNew(ThreadCom, NULL, NULL);
	mid_comQueue = osMessageQueueNew(MSGQUEUE_OBJECTS,sizeof(MSGQUEUE_OBJ_t),NULL);
	init_Usart();
	return(mid_comQueue);
	}
 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////77777
void myUSART_callback(uint32_t event) {
	if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) { 
		osThreadFlagsSet(tid_Thread_COM, RECEIVE_COMPLETE_FLAG); // A
	}
	if (event & ARM_USART_EVENT_SEND_COMPLETE) { 
		osThreadFlagsSet(tid_Thread_COM, SEND_COMPLETE_FLAG);
	}
}

static void init_Usart(void){				//TERATERM
  USARTdrv ->Initialize(myUSART_callback);	
	USARTdrv ->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 115200);
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
  USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7

static void ThreadCom (void *argument) {		//dato apunta al buffer
	char comando[9]="\r\n hola";
	USARTdrv->Send("\r\n Teraterm Configurado", 24);
	osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAny, osWaitForever);

	USARTdrv->Send(comando, 9);
	osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAny, osWaitForever);
	osDelay(2000);
	
	while (1) {
		USARTdrv->Receive(&rxBuffer, 1);
		osThreadFlagsWait(RECEIVE_COMPLETE_FLAG, osFlagsWaitAny, 2000);
		procesarDatos();
			//procesarDatos();
			//convertirBytesRxACaracter(rxBuffer);
		  //cleanRxbuff();
		  //guardo buffer
			//borro buffer
  }
}	

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7

static int inicioEncontrado = 0; // Bandera para indicar si se ha encontrado el inicio (01)
static uint32_t i;
static void procesarDatos(void) {
  if (rxBuffer == 0x01) { 
		inicioEncontrado = 1; // Se encontró el inicio
   }
  if (inicioEncontrado) { 
		bufferDefinitivo[indexDefinitivo] = rxBuffer; // Agrega el byte actual al buffer
    indexDefinitivo++;
    if (rxBuffer == 0xFE)	{ // Si se encuentra el final (FE), se termina la recolección y se procesa la información
    // Procesar datos en bufferDefivo
    inicioEncontrado = 0; // Reinicia la bandera del inicio
		procesarCasosRx();
		//procesarCasosRx(bufferDefinitivo);
    }
	}
}
		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////7/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void procesarCasosRx(void) {		
	if ((bufferDefinitivo[1] == 0x20 && indexDefinitivo==0x0C) || (bufferDefinitivo[1] == 0x25 && indexDefinitivo==0x08)|| (bufferDefinitivo[1] == 0x55 && indexDefinitivo==0x04) || (bufferDefinitivo[1] == 0x60 && indexDefinitivo==0x04)){ 
		USARTdrv->Send("\r\n mensajeValido",19);
		osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAny, osWaitForever);
		if (bufferDefinitivo[1] == 0x20&& indexDefinitivo==0x0C) {//horas min, seg
			msgRx.Id = 1;	
			msgRx.hora = (bufferDefinitivo[3] - '0') * 10 + (bufferDefinitivo[4] - '0');
			msgRx.minutos= (bufferDefinitivo[6] - '0') * 10 + (bufferDefinitivo[7] - '0');
			msgRx.segundos= (bufferDefinitivo[9] - '0') * 10 + (bufferDefinitivo[10] - '0');							
		} else if (bufferDefinitivo[1] == 0x25 && indexDefinitivo==0x08) {
			msgRx.Id = 2;//temperaturass
			//creamos la cola con el id 
			msgRx.temp = (bufferDefinitivo[3] - '0') * 10.0f + (bufferDefinitivo[4] - '0') + (bufferDefinitivo[5] == '.' ? 0.1f * (bufferDefinitivo[6] - '0') : 0.0f);							
		} else if (bufferDefinitivo[1] == 0x55 && indexDefinitivo==0x04) {
			msgRx.Id = 3;//peticion de medidas	
			//el receptor en principal mandara en el case 3 del depuracion, las medidas que tenga
		} else if (bufferDefinitivo[1] == 0x60 && indexDefinitivo==0x04) {
			msgRx.Id = 4;//borrar								
		}
		respuesta(); 
		osMessageQueuePut(mid_comQueue, &msgRx, 0U, osWaitForever);//se borrara las medidas del buffer circular
		indexDefinitivo=0;
		cleanRxBuff();
	}			
	else{
    USARTdrv->Send("\r\n mensajeInValido, debe contener 01 y Fe y un comando entre 20, 25, 55, 60", 80);
    osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAny, osWaitForever);
    osDelay(2000);
  }		
}
	
	////////////////////////////////////////////////////////RESPUESTA, se responde, en el caso 3 se piden medidas, limpiamos el bufferDefinitivo[1]//////////////////////////////////////////////////////////////////////////////////////////////

static void respuesta(void)	{	
  switch (msgRx.Id) { 
		case 1: // Respuesta puesta en hora//imprimir sin hacer el put
			snprintf(respPuestaHora, 31, "\r\n x01 xDF x0C %02d:%02d:%02d 0xFE", msgRx.hora, msgRx.minutos, msgRx.segundos);
			USARTdrv->Send(respPuestaHora, 31);
			osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAll, osWaitForever);
			//limpiar buffer
		break;
    case 2: // Respuesta referencia de temperatura imprimir sin hacer el put
			snprintf(respuestareftemp, 23, "\r\n 0x01 0xDA x08 %f 0xFE", msgRx.temp);//aqui poner las dos variables de Temp
			USARTdrv->Send(respuestareftemp, 23);
			osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAll, osWaitForever);
		break;
    case 3: // Devuelve el array de temperaturas7
			//!!!!! como coño hago eso el Get
			osMessageQueueGet(mid_comQueue, msgRx.medidas, 0U, osWaitForever);//se borrara las medidas del buffer circular
			for (r = 0; r < 10; r++){ 
				snprintf(medida, 40, "\r\n x01 xCA x05 0x%d 0xFE", msgRx.medidas[1]);//esto es un elemento, en mi array deberia meter todas mis medidas del buffer circular e ir sacandolas
				USARTdrv->Send(medida, 40);//ajustar esto que no es asi
				osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAll, osWaitForever);
			}
    break;						
    case 4: // Devuelve comando borrar medidas
			//borrar_medidas_Temp();
			snprintf(Borrrarmedidas, 20, "\r\n x01 x9F x04 0xFE ");
			USARTdrv->Send(Borrrarmedidas, 20);
			osThreadFlagsWait(SEND_COMPLETE_FLAG, osFlagsWaitAll, osWaitForever);
			//limpiarbufferdefinitivo
		break;
    default:
    break;
    }
}	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void cleanRxBuff(void){
	int i;
	for(i = 0; i < (12); i++){
		bufferDefinitivo[i] = 0;
	}
}
