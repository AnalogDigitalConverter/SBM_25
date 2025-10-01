/*P5_2 ADC 24/NOV/24*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
 *      Thled3 'Thread_led3'
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thled3;                        // thread id

static void Thled3(void *argument);
static void Led3_init(void);
int Init_Thled3(void);

int Init_Thled3 (void) {
  tid_Thled3 = osThreadNew(Thled3, NULL, NULL);
  if (tid_Thled3 == NULL) {
    return(-1);
  }

  return(0);
}

static void Thled3 (void *argument) {
  Led3_init();
  while (1) {
    // Insert thread code here...
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
    osDelay(287);
    osThreadYield();                            // suspend thread
  }
}

static void Led3_init(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

