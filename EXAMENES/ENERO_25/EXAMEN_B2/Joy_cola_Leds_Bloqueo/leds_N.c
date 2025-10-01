#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "leds_N.h"
#include "joy.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_leds_N;                        // thread id
extern osMessageQueueId_t id_joy_MsgQueue;
 
void leds_N (void *argument);                   // thread function
 
int Init_leds_N (void) {
 
  tid_leds_N = osThreadNew(leds_N, NULL, NULL);
  if (tid_leds_N == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void leds_N (void *argument) {
  InitLeds();
	MSG_JOY joy_prin;
	
  while (1) {
    //status = osThreadFlagsWait(0x0F, osFlagsWaitAny,osWaitForever);
		if (osOK == osMessageQueueGet(id_joy_MsgQueue, &joy_prin, 0, osWaitForever)){
    
    switch (joy_prin.button){
      case 'L':
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
        break;
        
      case 'R':
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
        break;
      
      case 'U':
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
        break;
        
      case 'D':
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
        break;
			
			 case 'C':
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
        break;
      }
		}
  
  }
}

void InitLeds(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
}
