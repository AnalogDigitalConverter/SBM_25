#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

//typedef enum{state1,state2}state_t
//static state_t state


extern TIM_HandleTypeDef tim2;
extern TIM_HandleTypeDef tim3;
extern TIM_HandleTypeDef tim4;
extern TIM_HandleTypeDef tim5;
extern TIM_HandleTypeDef tim6;
extern TIM_HandleTypeDef tim7;

uint8_t estado = 0;
uint8_t secuencia = 0;

void EXTI15_10_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
 if(GPIO_PIN_13){
   secuencia = (0 == secuencia) ? 1 : 0;
 }
}

void NMI_Handler(void)
{
}

void TIM2_IRQHandler(){
  HAL_TIM_IRQHandler(&tim2);
}

void TIM3_IRQHandler(){
  HAL_TIM_IRQHandler(&tim3);
}

void TIM4_IRQHandler(){
  HAL_TIM_IRQHandler(&tim4);
}

void TIM5_IRQHandler(){
  HAL_TIM_IRQHandler(&tim5);
}
void TIM6_DAC_IRQHandler(){
  HAL_TIM_IRQHandler(&tim6);
}
void TIM7_IRQHandler(){
  HAL_TIM_IRQHandler(&tim7);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if (htim -> Instance == TIM6){
    if (secuencia == 0) {
    switch(estado) {
      case 0:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 1;
        break;
      case 1:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 2;
        break;
      case 2:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 3;
        break;
      case 3:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
      estado = 1;
      break;
    }
  } else {
    switch(estado) {
      case 0:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 3;
        break;
      case 1:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 3;
        break;
      case 2:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      estado = 1;
        break;
      case 3:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
      estado = 2;
      break;
    }
  }
    //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
  } else {
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
  }
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


