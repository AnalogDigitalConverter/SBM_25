/*P5_3 ADC 24/NOV/24*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define LED2 0b01
#define LED3 0b10

/*----------------------------------------------------------------------------
*      Thled2 'Thread_led2'
*---------------------------------------------------------------------------*/

osThreadId_t tid_Thled2;                        // thread id
extern osThreadId_t tid_Thled3;

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

static void Thled2 (void *argument)
{
  Led2_init();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

  int cnt_2 = 0;

  osThreadFlagsWait(LED2, osFlagsWaitAll, osWaitForever);

  while (1) {
    // Insert thread code here...
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    osDelay(137);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    osDelay(137);

    cnt_2++;
    if (cnt_2 == 15)
    {
      osThreadFlagsSet(tid_Thled3, LED3);
    }
    else if (cnt_2 == 20)
    {
      cnt_2 = 0;
      osThreadFlagsWait(LED2, osFlagsWaitAll, osWaitForever);
    }
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

