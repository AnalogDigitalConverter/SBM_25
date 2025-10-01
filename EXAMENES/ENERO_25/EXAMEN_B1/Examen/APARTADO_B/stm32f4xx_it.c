
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
#include <stdbool.h> //******* PARA QUE RECONOZCA EL BOOL ***/////

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif



extern uint8_t cont;
uint8_t seg=0; // representa medio segundo
uint8_t segundo=0; // representa un segundo
uint16_t estado=0; 
uint8_t pulsaciones=0;
void controlDeEstado(void);


void TIM7_IRQHandler(void){ //Primera capa de la interrupcion del Timer
	HAL_TIM_IRQHandler(&htim7); //Esta funcion ya esta definida
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim -> Instance == TIM7){
		seg++;
    //para la depuracion 
    if(seg % 2) segundo++;
    //
    controlDeEstado();

	}
}


void controlDeEstado(void){
  switch (estado){
        case 0: // los 3 led encendidos
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,  GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
          
          if(seg>6) estado=1; //3 segundos
        break;
        
        case 1:// Led 1 y 2 toggle y 3 apagadp
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
          HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
          HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
          
          if(seg>12) estado=2; // 6 segundos
        break;
        
        case 2:
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
          HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
          
          if(seg>18) estado=3; //9 segundos
         break;
        
        case 3:
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          HAL_TIM_Base_DeInit(&htim7);
          HAL_TIM_Base_Stop_IT(&htim7);
        break;
      }
}


void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	
}

	
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin ==GPIO_PIN_13){
      pulsaciones++;
      if(estado==0 && pulsaciones>5){
        estado=3;
        controlDeEstado();
      }

	}
}
	
	
	
	
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


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
