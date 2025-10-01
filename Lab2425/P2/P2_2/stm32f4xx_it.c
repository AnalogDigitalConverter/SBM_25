/* P2_2 - ANTONIO DONA CANDELA - 5OCT24 */

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
extern TIM_HandleTypeDef htim7;
/**
  * @brief  This function strictly handles TIM7 interrupt request.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);
}

/**
  * @brief  This function handles ALL timer interrupt request actions and
  * 		resets each interrupt flag.
  * @param  htim: pointer to TIM_HandleTypeDef to be handled.
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//Case: TIM7 = 1.5s LED Toggle
	if(htim->Instance == TIM7)
	{
		//LD1-Green(Pin0), LD2-Blue(Pin7), LD3-Red(Pin14)
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	
	/*Disabled code****************************
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	*******************************************/
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
