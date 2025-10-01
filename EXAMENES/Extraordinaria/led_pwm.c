#include "cmsis_os2.h"                          // CMSIS RTOS header file
 

static TIM_HandleTypeDef htim1;
static TIM_OC_InitTypeDef TIM_Channel_InitStruct;

extern osMessageQueueId_t PWM_Queue;
extern MSGQUEUE_PWM_t pwm_update;

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_LED_Th;                        // thread id
 
void LED_Th (void *argument);                   // thread function
 
int Init_LED_Th (void) {
  tid_LED_Th = osThreadNew(Thread, NULL, NULL);
  if (tid_LED_Th == NULL) {
    return(-1);
  }
  return(0);
}
 
void led_pwm_Th (void *argument) {

  //hw init
  init_pwm();

  //local variables
  uint16_t current_pwm = 0;

  while (1) {
    /*STATUS: OUTPUT*/
    //if(new_message)
    osMessageQueueGet(PWM_Queue, &pwm_update, NULL, 0U);

    //Update only if reqd.value changed
    if (current_pwm != pwm_update.width)
    {
      //pwm update and save
      current_pwm = pwm_timer_update();
    }
    osThreadYield();                            // suspend thread
  }
}

/******************FUNCTIONS*****************/
static uint16_t pwm_timer_update(void)
{
  //Struct field data update
  TIM_Channel_InitStruct.Pulse = pwm_update.width;

  //Stop current PWM delivery
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

  //PWM Channel Update & Start
  HAL_TIM_PWM_ConfigChannel(&htim1, &TIM_Channel_InitStruct, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  return pwm_update.width;

}

/*-----------------hardware init*********************/
static void init_pwm(void)
{
  static GPIO_InitTypeDef GPIO_InitStruct;

  /************PWM at pin PE9*************/
  //Enable clock on port E
  __HAL_RCC_GPIOE_CLK_ENABLE();

  //Pin PE9
  GPIO_InitStruct.Pin       = GPIO_PIN_9;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /************PWM Timer Config*************/
  //GPIO_AF1_TIM1 & Scaler Set
  htim1.Instance           = TIM1;
  htim1.Init:Prescaler     = 2099
  htim1.Init.Period        = 79;
  htim1.Init.CounterMode   = TIM_COUNTERMODE_UP;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim1);

  //Timer Channel Config & Start
  TIM_Channel_InitStruct.OCMode     = TIM_OCMODE_PWM1;
  TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
  TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE

  HAL_TIM_PWM_ConfigChannel(&htim1, &TIM_Channel_InitStruct, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1; TIM_CHANNEL_1);
}
