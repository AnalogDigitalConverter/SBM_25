/* CÓDIGO TOP - ANTONIO DONA CANDELA - 16OCT24 */
/* Includes -----------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

/* Public Types -------------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;

extern TIM_OC_InitTypeDef TIM_Channel_InitStruct;

extern uint16_t periodo_tim2;

/* Private variables --------------------------------------------------------*/
static uint16_t val_0, val_1, diffCapture, frequency, overflow_counter;
static uint8_t 	edge_rise = 1;

/*****************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                  */
/*****************************************************************************/

/**
  * @brief  This function strictly handles General Purpose TIM2 PWM capable
  * 		(APB1 45MHz x2 Max.Freq) interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim2);
}

/**
  * @brief  This function strictly handles Input Capture and Period Elapsed
  *         Callbacks from IC TIM3 (APB1 45MHz x2 Max.Freq).
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(){
  HAL_TIM_IRQHandler(&htim3);
}

/**
  * @brief  This function strictly handles Basic TIM7 (APB1 45MHz x2 Max.Freq)
  *  		interrupt request.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);
}

/**
  * @brief  This function handles ALL External interrupt request actions and
  * 		resets each interrupt flag.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler (void){
	HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
}

/*****************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Callbacks                 */
/*****************************************************************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  /*
  if(htim->Instance == TIM2){

  }
  */
  
  if (htim -> Instance == TIM3){
    overflow_counter++;
  }
  
  /*
  if(htim->Instance == TIM7){
  
  }
  */
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM3)
  {
    if(edge_rise == 1)
	{
      //init count reference
      val_0 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
	  overflow_counter = 0;

      edge_rise = 2;
    }
	else if(edge_rise == 2)
    {
      val_1 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + (overflow_counter * htim3.Init.Period);
      diffCapture = val_1 - val_0;
	  
	  edge_rise = 1;

      //Discard null values of low sampling freq and null overflow.
      frequency = ((HAL_RCC_GetPCLK1Freq() / diffCapture) != 0) ? (HAL_RCC_GetPCLK1Freq() / diffCapture) : frequency;
    }
  }
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){
  /**CHULETA ***********************
  * if (GPIO_Pin == GPIO_PIN_?){
  *  //lógica de excepcion del botón
  * } 
  *********************************/
  
  //User Button Interrupt Logic
  if (GPIO_Pin == GPIO_PIN_13)
  {
    /*Timer Stop for Config/Parameter Update*/
    HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);	
    
    //Period update from 2kHz to 5kHz -> half duty cycle
    htim2.Init.Period    = (htim2.Init.Period == 41999) ? 16799 : 41999;

    /*Timer Register Config*/
    HAL_TIM_OC_Init(&htim2);

    /*Timer Register & Channel HW Config LOAD*/
    HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4);

    /*Timer_2 CHA_4 Init*/
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
  }

  //JOY-UP(PE10)-> freq x2 -> period -> /2 (mejor valor fijo, error prevent.)
  if (GPIO_Pin == GPIO_PIN_10)
  {
    /*Timer Stop for Config/Parameter Update*/
    HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);

    //Period half, doubling freq output
    htim2.Init.Period    = (htim2.Init.Period > 1)? htim2.Init.Period >> 1 : 0;

    /*Timer Register Config*/
    HAL_TIM_OC_Init(&htim2);

    /*Timer Register & Channel HW Config LOAD*/
    HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4);

    /*Timer_2 CHA_4 Init*/
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
  }

  //JOY-DN(PE12) -> freq /2 -> period -> *2 (mejor valor fijo, error prevent.)
  if (GPIO_Pin == GPIO_PIN_12)
  {
    /*Timer Stop for Config/Parameter Update*/
    HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);

    //Period double, half freq output
    htim2.Init.Period    = (htim2.Init.Period < 32768)? htim2.Init.Period << 1 : 65535;

    /*Timer Register Config*/
    HAL_TIM_OC_Init(&htim2);

    /*Timer Register & Channel HW Config LOAD*/
    HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4);

    /*Timer_2 CHA_4 Init*/
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
  }
}
/************************ ADC *****END OF FILE********************************/
/*
*
*
*
*
*
*
*/

/*****************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                        */
/*****************************************************************************/

void NMI_Handler(void)
{
}
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}
#ifndef RTE_CMSIS_RTOS2_RTX5
void SVC_Handler(void)
{
}
#endif
void DebugMon_Handler(void)
{
}
#ifndef RTE_CMSIS_RTOS2_RTX5
void PendSV_Handler(void)
{
}
#endif
#ifndef RTE_CMSIS_RTOS2_RTX5
void SysTick_Handler(void)
{
  HAL_IncTick();
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE***/
