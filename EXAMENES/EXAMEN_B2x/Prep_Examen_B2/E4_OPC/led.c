#include "cmsis_os2.h"        //Para manejar el RTOS
#include "stm32f4xx_hal.h"    //Para manejar el HAL

#include "led.h"

static osThreadId_t id_Th_led;

int init_Th_led(void);
osThreadId_t get_id_Th_led(void);

static void Th_led(void *arg);
static void led_init(void);

static int Init_MsgQueue_joy(void);
static osMessageQueueId_t id_MsgQueue_led;
osMessageQueueId_t get_id_MsgQueue_led(void);

osTimerId_t tim_2seg;
osTimerId_t tim_4Hz;
osTimerId_t tim_2Hz;
uint8_t cont=0;
uint8_t estado=1;


static int Init_MsgQueue_led(void){
	id_MsgQueue_led = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_LED), NULL);
	if(id_MsgQueue_led == NULL)
		return(-1);
	
	return (0);
}

osMessageQueueId_t get_id_MsgQueue_led(void){
	return id_MsgQueue_led;
}

int init_Th_led(void){
  led_init();
	id_Th_led = osThreadNew(Th_led, NULL, NULL);
  
	if(id_Th_led == NULL)
		return(-1);
  
	return(Init_MsgQueue_led());
}

osThreadId_t get_id_Th_led(void){
 return id_Th_led;
}

static void tim_2seg_Callback(void* argument){  
	osTimerStop(tim_4Hz);
}
static void tim_4Hz_Callback(void* argument){  
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}
static void tim_2Hz_Callback(void* argument){  
	switch(estado){
					case 1:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
						estado++;
						break;
					case 2:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
						estado++;
						break;
					case 3:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
						estado++;
						break;
					case 4:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
						estado++;
						break;
					case 5:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
						estado++;
						break;
					case 6:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
						estado++;
						break;
					case 7:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
						estado++;
						break;
					case 8:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
						estado++;
						break;
					case 9:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
						estado++;
						break;
					case 10:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
						estado++;
						break;
					case 11:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
						estado++;
						break;
					case 12:
						HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
						estado=1;
						break;
					
				}
}

static void Th_led(void *argument){ // funcion del hilo
	uint32_t flag;
	tim_2seg = osTimerNew(tim_2seg_Callback, osTimerOnce, (void*)0, NULL); 
	tim_4Hz = osTimerNew(tim_4Hz_Callback, osTimerPeriodic, (void*)0, NULL); 
	tim_2Hz = osTimerNew(tim_2Hz_Callback, osTimerPeriodic, (void*)0, NULL);
  while(1){
		flag = osThreadFlagsWait(MLT4 | STP , osFlagsWaitAny, osWaitForever);

		if(flag & MLT4){
			osTimerStart(tim_2seg, 2000);
			osTimerStart(tim_4Hz, 125);
       
    }
		if(flag & STP){
			osTimerStart(tim_2Hz, 250);
			
		}
		
	}

 
}

static void led_init(void){  //Inicializar parametros el LED
  
  static GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // led verde (LED1)
	
  GPIO_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //led azul (LED2)
	
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // led rojo (LED3)
}
