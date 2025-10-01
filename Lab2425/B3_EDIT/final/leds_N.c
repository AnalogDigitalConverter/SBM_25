#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "leds_N.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_leds_N;                        // thread id
 
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
  uint16_t status;
  while (1) {
    status = osThreadFlagsWait(0x0F, osFlagsWaitAny,osWaitForever);
    
    switch (status){
      case LED1_ON:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
        break;
        
      case LED2_ON:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
        break;
      
      case LED3_ON:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
        break;
        
      case LEDS_OFF:
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
        break;
    }
  
  }
}

void InitLeds(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull 	= GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
}
