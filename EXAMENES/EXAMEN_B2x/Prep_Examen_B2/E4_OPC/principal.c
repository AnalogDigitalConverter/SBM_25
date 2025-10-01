#include "cmsis_os2.h"        //Para manejar el RTOS
#include "stm32f4xx_hal.h"    //Para manejar el HAL
#include <stdio.h>
#include <string.h>
#include "principal.h"

static osThreadId_t id_Th_principal;

int init_Th_principal(void);

static void Th_principal(void *arg);

uint8_t segundo=10;
uint8_t minuto=0;
osTimerId_t tim_seg;
static uint32_t flag;

int init_Th_principal(void){
	init_Th_led();
	init_Th_lcd();
	
	id_Th_principal = osThreadNew(Th_principal, NULL, NULL);
  
	if(id_Th_principal == NULL)
		return(-1);
  
  return 0;
}
osThreadId_t get_id_Th_principal(void){
return id_Th_principal;
}

static void tim_seg_Callback(void *argument){
	osThreadFlagsSet(get_id_Th_principal(), SEG);
}

static void Th_principal(void *argument){ // funcion del hilo
  //timer para los segundos
	MSGQUEUE_OBJ_LCD msg_lcd;
	tim_seg = osTimerNew(tim_seg_Callback, osTimerPeriodic, (void *)0, NULL);
	osTimerStart(tim_seg, 1000U);
	
  while(1){
		flag = osThreadFlagsWait(_____, osFlagsWaitAny, 300U);
    
		if(flag &){ 
  
		} 
  }

}



