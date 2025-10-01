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

osThreadId_t get_id_Th_Cola(void);

static void Th_Cola(void *arg);


int init_Th_Cola(void){

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
  	int rep =0;
	
	  for (rep = 0; rep < 5; rep++) {
	 	LCD_reset();
		LCD_Nada();
	  LCD_init();
	  LCD_update();
			
		positionL1 = 10 + rep * 5; 
			symbolToLocalBuffer_L1('1');
			symbolToLocalBuffer_L1('2');
			symbolToLocalBuffer_L1('3');
			symbolToLocalBuffer_L1('4');
		  LCD_update();
		osDelay(1000);
			
 
		positionL2 = 38 + rep * 5; 
		symbolToLocalBuffer_L2('5');
		symbolToLocalBuffer_L2('6');
		update_texto2(texto2);
	  LCD_update();
		osDelay(2000);
		
			}
		
			LCD_reset();
	    LCD_init();
		  LCD_Nada();
	    LCD_update();
		
  while(1){}
		
	}



