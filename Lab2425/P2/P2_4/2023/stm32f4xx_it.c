/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory overflow_counter this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */
	
/* Public Types --------------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern uint16_t periodo_tim2;
extern TIM_OC_InitTypeDef TIM_Channel_InitStruct;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint32_t val_0, val_1, diffCapture, frequency;
static uint8_t edge_rise = 1;
static uint16_t overflow_counter;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
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
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void TIM3_IRQHandler(){
  HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim -> Instance == TIM3)
    overflow_counter++;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM3){
		
    if(edge_rise == 1){
			overflow_counter = 0;
      val_0 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
      edge_rise = 2;
			
    }else if(edge_rise == 2){
      val_1 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1) + (overflow_counter * htim3.Init.Period);
      edge_rise = 1;
      diffCapture = val_1 - val_0;
			
			//Discard null values of low sampling freq and null overflow.
      frequency = ((HAL_RCC_GetPCLK1Freq() / diffCapture) != 0)?(HAL_RCC_GetPCLK1Freq() / diffCapture):frequency;
    }
  }
}

void EXTI15_10_IRQHandler (void){
	HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){
		//TIM_OC_InitTypeDef TIM_Channel_InitStruct;		//OJO SE TIENE QUE LLAMAR IGUAL QUE EN MAIN, COMPROBADO

	/*	SI 3+ CASOS: SWITCH
//    if (GPIO_Pin == GPIO_PIN_13){
//			if (periodo_tim2 == 41999)
//					periodo_tim2 = 16799;			//2500Hz
//			else if (periodo_tim2 == 16799)
//					periodo_tim2 = 10549;			//~3981Hz
//			else if (periodo_tim2 == 10549)
//					periodo_tim2 = 4199;			//~10KHz
//			else
//					periodo_tim2 = 41999;		*/
	
    if (GPIO_Pin == GPIO_PIN_13){			//Case de interrupciones externas para controlar actuación por botón u otras señales. Ver Joystick
			switch(periodo_tim2){
				case 41999:
						periodo_tim2 = 16799;			//2500Hz
				break;
				
				case 16799:
					periodo_tim2 = 10549;			//~3981Hz
				break;
				
				case 10549:
					periodo_tim2 = 4199;			//~10KHz
				break;
				
				default:
					periodo_tim2 = 41999;			
		}
			
			/*Timer Stop for Config/Parameter Update*/
			HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_4);			
			
			/*Timer Register Config*/
			htim2.Init.Period = periodo_tim2;
			HAL_TIM_OC_Init(&htim2);								//Permite TIM Config
	
			/*Timer Register & Channel HW Config LOAD*/
			HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4); //Carga TIM Config en CHA_4
			
			/*Timer_2 CHA_4 Init*/
			HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
		}
}
/**
  * @}
  */ 

/**
  * @}
  */
