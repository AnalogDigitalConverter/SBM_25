/* P2_3 - ANTONIO DONA CANDELA - 5OCT24 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void SVC_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS2_RTX5
void SysTick_Handler(void)
{
  HAL_IncTick();
}
#endif

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/******************************************************************************/
extern uint16_t periodo_tim2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim7;
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
  * @brief  This function strictly handles General Purpose TIM2 PWM capable
  * 		(APB1 45MHz x2 Max.Freq) interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim2);
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

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){
  if (GPIO_Pin == GPIO_PIN_13){
	  
	/*Timer Stop for Config/Parameter Update*/
    HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);	
    
	//Period update from 1kHz to 2.5kHz
    htim2.Init.Period    = (htim2.Init.Period == 41999) ? 16799 : 41999;

    /*Timer Register Config*/
    HAL_TIM_OC_Init(&htim2);
	
    /*Timer Register & Channel HW Config LOAD*/
    HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4);
			
    /*Timer_2 CHA_4 Init*/
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
	}

	//UP (PE10) -> freq x2 -> period -> /2 (mejor valor fijo ó desplazamiento?)
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
    //DN (PE12) -> freq /2 -> period -> *2 (mejor valor fijo ó desplazamiento?)
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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
