/*P5_2 ADC 24/NOV/24*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
 *      Thled2 'Thread_led2'
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thled2;                        // thread id

static void Thled2(void *argument);
static void Led2_init(void);
int Init_Thled2 (void);

int Init_Thled2 (void) {
  tid_Thled2 = osThreadNew(Thled2, NULL, NULL);
  if (tid_Thled2 == NULL) {
    return(-1);
  }

  return(0);
}

static void Thled2 (void *argument) {
  Led2_init();
  while (1) {
    // Insert thread code here...
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    osDelay(137);
    osThreadYield();                            // suspend thread
  }
}

static void Led2_init(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

