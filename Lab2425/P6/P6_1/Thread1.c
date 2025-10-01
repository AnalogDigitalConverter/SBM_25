/*P6_1 adc 24/NOV/24*/

#define JOY_R_FLAG 0x08
#define COUNTDOWN_3S 3000U


#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
/*----------------------------------------------------------------------------
*      Thread 1 'main_Thread'
*---------------------------------------------------------------------------*/

osThreadId_t tid_main_Thread;                        // thread id
void main_Thread (void *argument);                   // thread function


static uint8_t flags = 0;
static osTimerId_t CountDown_Timeout;
static void CountDown_Timeout_Callback (void* argument);
static void leds_Init(void);

int Init_main_Thread (void) {
  tid_main_Thread = osThreadNew(main_Thread, NULL, NULL);
  if (tid_main_Thread== NULL) {
    return(-1);
  }

  return(0);
}

void main_Thread (void *argument) {
  leds_Init();

  //LED1 G
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  //LED3 R
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  //ARRANCAR CUENTA
  CountDown_Timeout = osTimerNew(CountDown_Timeout_Callback, osTimerOnce, (void *)0, NULL);
  osTimerStart(CountDown_Timeout, COUNTDOWN_3S);


  while (1) {

    //wait any other tick
    flags = osThreadFlagsWait(JOY_R_FLAG, osFlagsWaitAny, 2);

    if(flags = JOY_R_FLAG)
    {
      osTimerStart(CountDown_Timeout, COUNTDOWN_3S);
    }

}


static void CountDown_Timeout_Callback (void* argument)
{
  //LED1 G
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  //LED3 R
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

  while(1)
  {
    //lock
  }

}


static void leds_Init(void){
  static GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
