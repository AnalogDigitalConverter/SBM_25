/*P5_3 ADC 24/NOV/24*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define LED2 0b01
#define LED3 0b10

/*----------------------------------------------------------------------------
*      Thled3 'Thread_led3'
*---------------------------------------------------------------------------*/

osThreadId_t tid_Thled3;                        // thread id
extern osThreadId_t tid_Thled2;

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

static void Thled3 (void *argument)
{
  Led3_init();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

  int cnt_3 = 0;

  osThreadFlagsWait(LED2, osFlagsWaitAll, osWaitForever);

  while (1) {
    // Insert thread code here...
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    osDelay(287);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    osDelay(287)

    cnt_3++;
    if (cnt_3 == 25)
    {
      osThreadFlagsSet(tid_Thled2, LED2);
    }
    else if (cnt_3 == 30)
    {
      cnt_3 = 0;
      osThreadFlagsWait(LED3, osFlagsWaitAll, osWaitForever);
    }
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

