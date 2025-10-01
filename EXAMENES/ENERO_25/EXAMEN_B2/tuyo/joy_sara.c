#include "joy.h"

/****
MODULO JOYSTICK:
Módulo encargado de detectar e identificar una pulsación en el joystick
Entrada - Ninguna
Salida - Método de sincronización: Message Queue
				Cada vez que se detecta una pulsación se envía un mensaje a una
				cola de mensajes con información de la tecla pulsada y si la pulsación
				ha sido normal o larga (1 seg).
Ficheros - joystick.c y joystick.h 
****/

static osThreadId_t id_Th_joy;

int init_Th_joy(void);
osThreadId_t get_id_Th_joy(void);
osMessageQueueId_t get_id_MsgQueue_joy(void);

static void init_joy(void);
static int Init_MsgQueue_joy(void);
static osMessageQueueId_t id_MsgQueue_joy;
static void Th_joy(void *arg);

static MSGQUEUE_OBJ_JOY msg;

static uint32_t flag;         //Variable para almacenar las flags (REB_FLAG, IRQ_FLAG, CHECK_FLAG)
osTimerId_t tim_reb;		  		//Timer encargado de gestionar los rebotes y duracion, durará 50 ms
osTimerId_t tim_pul;
dir_t comprobarPines(void); 
static uint8_t cnt;

static int Init_MsgQueue_joy(void){
	id_MsgQueue_joy = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_JOY), NULL);
	if(id_MsgQueue_joy == NULL)
		return(-1);
	
	return (0);
}

osMessageQueueId_t get_id_MsgQueue_joy(void){
	return id_MsgQueue_joy;
}

int init_Th_joy(void){
	init_joy();
	id_Th_joy = osThreadNew(Th_joy, NULL, NULL);
  
	if(id_Th_joy == NULL)
		return(-1);
  
	return(Init_MsgQueue_joy());
}

osThreadId_t get_id_Th_joy(void){
	return id_Th_joy;
}

static void tim_reb_Callback(void* argument){  
		osThreadFlagsSet(get_id_Th_joy(), REB);
	  
}

static void tim_puls_Callback(void* argument){  
		if(cnt<=20){
			if(comprobarPines()!=NONE){ //sigue pulsado
				cnt++;
			}
			else{
				msg.dur=0;
				osThreadFlagsSet(get_id_Th_joy(), CHECK);
			}
		}
		else{
			msg.dur=1;
			osThreadFlagsSet(get_id_Th_joy(), CHECK);
		}


}
static void Th_joy(void *argument){ // funcion del hilo
  tim_reb = osTimerNew(tim_reb_Callback, osTimerOnce, (void*)0, NULL); 
	tim_pul = osTimerNew(tim_puls_Callback, osTimerPeriodic, (void*)0, NULL); 
  while(1){
		flag = osThreadFlagsWait(REB | IRQ | CHECK, osFlagsWaitAny, osWaitForever);
		
		if(flag & IRQ){
			osTimerStart(tim_reb, 50U);
		}
		if(flag & REB){
				osTimerStart(tim_pul, 50U);
				cnt=0;
		}
		if(flag & CHECK){
			osTimerStop(tim_pul);
			osMessageQueuePut(get_id_MsgQueue_joy(), &msg, 0U, 0U);
		}
	}
}

dir_t comprobarPines(void){
	dir_t pin;
	
	if     (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) pin = DOWN;  
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) pin = LEFT;  
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET) pin = MIDDLE;
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET)	pin = UP;    
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) pin = RIGHT; 
	else pin=NONE;
	
	return pin;
}

static void init_joy(void){
	
	static GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;					// DOWN
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;					// LEFT
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;					// MIDDLE
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;					// UP
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;					// RIGHT
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
}

  void EXTI15_10_IRQHandler(void){
    
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15); 
		
  }
  
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		msg.dir= comprobarPines(); 	// compruebo si siguen activa
    osThreadFlagsSet(get_id_Th_joy(), IRQ);
		
  }

