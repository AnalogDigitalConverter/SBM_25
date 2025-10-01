#include "cmsis_os2.h"        //Para manejar el RTOS
#include "stm32f4xx_hal.h"    //Para manejar el HAL

#include "led.h"

static osThreadId_t id_Th_led;
static osMessageQueueId_t id_MsgQueue_led;
static int Init_MsgQueue_led(void);
osMessageQueueId_t get_id_MsgQueue_led(void);
uint8_t ok=1;
int init_Th_led(void);
static void joy_init(void);

osThreadId_t get_id_Th_led(void);

static void Th_led(void *arg);
static void led_init(void);

osTimerId_t tim_reb;


int init_Th_led(void){
  led_init();
  joy_init();
	id_Th_led = osThreadNew(Th_led, NULL, NULL);
  
	if(id_Th_led == NULL)
		return(-1);
  

  return(Init_MsgQueue_led());
}

osThreadId_t get_id_Th_led(void){
 return id_Th_led;
}


static int Init_MsgQueue_led(void){
	id_MsgQueue_led = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_LED), NULL);
	if(id_MsgQueue_led == NULL){
		return(-1);
	}else{
		return (0);
	}
	
}

osMessageQueueId_t get_id_MsgQueue_led(){
return id_MsgQueue_led;
}

static void tim_reb_Callback(void *argument){
	ok=0;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  osThreadYield();
}


static void Th_led(void *argument){ // funcion del hilo
   
   MSGQUEUE_OBJ_LED msg_led;
	 osStatus_t status;
  
   tim_reb = osTimerNew(tim_reb_Callback, osTimerOnce, (void *)0, NULL);
   osTimerStart(tim_reb, 5000U);
  while(1){
    status = osMessageQueueGet(get_id_MsgQueue_led(), &msg_led, NULL, 100U);
		if(status==osOK){
      osTimerStart(tim_reb, 5000U);
			switch(msg_led.dir){
				case 1: // UP 1 0 1
          if(ok==1){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          }
					break;
				case 2: //DOWN 1 0 0
          if(ok==1){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          }
					break;
				case 4: // LEFT 1 1 1
          if(ok==1){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          }
            
					break;
				case 8: // RIGHT 1 1 0
          if(ok==1){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          }
					break;
				case 16: // MIDDLE 0 1 1 
          if(ok==1){
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          }
					break;
			}
			
			
		}
	}

 
}
static void joy_init(void){  //Inicializar parametros el LED
  
  static GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
  
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;			//UP
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;			//RIGHT		
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;			//DOWN
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;			//LEFT
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;			//MIDDLE
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	//---- LEDS 
	 __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // led verde
	
  GPIO_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //led azul
	
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // led rojo
	
	  
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}
static void led_init(void){  //Inicializar parametros el LED
  
  static GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); // led verde
	
  GPIO_InitStruct.Pin = GPIO_PIN_7;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET); //led azul
	
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // led rojo
}
