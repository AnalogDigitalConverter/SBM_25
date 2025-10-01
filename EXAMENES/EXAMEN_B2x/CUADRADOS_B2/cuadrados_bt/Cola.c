#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Cola.h"
#define EXIT_FLAG 0x01

static osThreadId_t id_Th_Cola;
static osMessageQueueId_t id_MsgQueue_Cola;
static int Init_MsgQueue_Cola(void);
osMessageQueueId_t get_id_MsgQueue_Cola(void);

int init_Th_Cola(void);

static char texto1[32];
static char texto2[32];

extern uint16_t positionL1 ;
extern uint16_t positionL2;
static void GPIO_Init(void);

osThreadId_t get_id_Th_Cola(void);

static void Th_Cola(void *arg);


int init_Th_Cola(void){
GPIO_Init();
	id_Th_Cola = osThreadNew(Th_Cola, NULL, NULL);
  
	if(id_Th_Cola == NULL)
		return(-1);
  

  return(Init_MsgQueue_Cola());
}

osThreadId_t get_id_Th_Cola(void){
 return id_Th_Cola;
}


static int Init_MsgQueue_Cola(void){
	id_MsgQueue_Cola = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_Cola), NULL);
	if(id_MsgQueue_Cola == NULL){
		return(-1);
	}else{
		return (0);
	}
	
}

osMessageQueueId_t get_id_MsgQueue_Cola(){
return id_MsgQueue_Cola;
}



static void Th_Cola(void *argument){ // funcion del hilo
  MSGQUEUE_OBJ_Cola msg_Cola;
	 osStatus_t status;
	
  LCD_reset();
		LCD_Nada();
	  LCD_init();
	  LCD_update();
			positionL1=10;
			symbolToLocalBuffer_L1('1');
			symbolToLocalBuffer_L1('2');
			symbolToLocalBuffer_L1('3');
			symbolToLocalBuffer_L1('4');
		  LCD_update();
     	positionL2=38;
		symbolToLocalBuffer_L2('5');
		symbolToLocalBuffer_L2('6');
	  LCD_update();
	
  while(1){
	 status = osMessageQueueGet(get_id_MsgQueue_Cola(), &msg_Cola, NULL, 100U);
    	if(status==osOK){
				 	if (msg_Cola.pulsador%2==0){
	
		LCD_reset();
		LCD_Nada();
	  LCD_init();
	  LCD_update();
			
			symbolToLocalBuffer_L1('1');
			symbolToLocalBuffer_L1('2');
			symbolToLocalBuffer_L1('3');
			symbolToLocalBuffer_L1('4');
		  LCD_update();
			
		symbolToLocalBuffer_L2('5');
		symbolToLocalBuffer_L2('6');
	  LCD_update();
					}
						
		else{
			
		LCD_reset();
		LCD_Nada();
	  LCD_init();
	  LCD_update();
     positionL1=10;
		symbolToLocalBuffer_L1('5');
		symbolToLocalBuffer_L1('6');
		 LCD_update();
			   	positionL2=38;
			symbolToLocalBuffer_L2('1');
			symbolToLocalBuffer_L2('2');
			symbolToLocalBuffer_L2('3');
			symbolToLocalBuffer_L2('4');
	   
	  LCD_update();
		        }
		
					}
			}
	}
		
	

static void GPIO_Init(void){
	static GPIO_InitTypeDef GPIO_InitStruct;
	//habilitar relojes
		__HAL_RCC_GPIOC_CLK_ENABLE();
	
	//habilitar interrupciones por el btUser
	  HAL_NVIC_SetPriority(EXTI15_10_IRQn,0,0); //los ceros son  las prioridades y subprioridad
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 

	//Inicializacion del boton User
	GPIO_InitStruct.Pull = GPIO_PULLDOWN; // SE MIRA EN EL ESQUEMA
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;// SE SCTIVA FLANCO DE SUBIDA
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}