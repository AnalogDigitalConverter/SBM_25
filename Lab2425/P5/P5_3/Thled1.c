/*P5_3 ADC 24/NOV/24*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define LED2 0b01
#define LED3 0b10

/*----------------------------------------------------------------------------
*      Thled1 'Thread_led'
*---------------------------------------------------------------------------*/

osThreadId_t tid_Thled1;                        // thread id
extern osThreadId_t tid_Thled2;

static void Thled1(void *argument);
static void Led1_init(void);
int Init_Thled1 (void);

int Init_Thled1 (void) {
  tid_Thled1 = osThreadNew(Thled1, NULL, NULL);
  if (tid_Thled1 == NULL) {
    return(-1);
  }

  return(0);
}

static void Thled1 (void *argument)
{
  Led1_init();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  int cnt_1 = 0;

  while (1) {
    // Insert thread code here...
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    osDelay(200);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    osDelay(800);

    (cnt_1 < 6)? cnt_1++;
    (cnt_1 == 5)? osThreadFlagsSet(tid_Thled2, Led2);
  }
}

static void Led1_init(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

