/*P5_1 ADC 24/NOV/24*/

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
*      Thled1 'Thread_led'
*---------------------------------------------------------------------------*/

osThreadId_t tid_Thled1;                        // thread id
static int Init_Thled1(void);

static void Thled1(void *argument);
static void Leds_init(void);


static int Init_Thled1 (void)
{
  tid_Thled1 = osThreadNew(Thled1, NULL, NULL);
    if (tid_Thled1 == NULL) {
      return(-1);
    }

    return(0);
}


static void Thled1(void *argument)
{
  Leds_init();
  while (1) {
    // Insert thread code here...
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    osDelay(200);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    osDelay(800);

    osThreadYield();                            // suspend thread
  }
}

static void Leds_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

  //LD0, LD1, LD2
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

