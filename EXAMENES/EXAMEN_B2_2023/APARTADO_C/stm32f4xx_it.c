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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

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
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/*AÑADIR THREAD.H A MAIN.H. ASÍ EL ARCHIVO CONOCERÁ LA COLA DE MENSAJES.
JOY_TX_MSG ES MENSAJE PROPIO, ASÍ QUE ES LOCAL. LA COLA ES EL ELEMENTO COMÚN
*/
extern osMessageQueueId_t id_joy_MsgQueue;
MSG_JOY joy_tx_msg;

void EXTI15_10_IRQHandler (void)
{
  /*
   *     External Interrupt buttons:
   *         PB10: Joystick UP
   *         PB11: Joystick DERECHA
   *         PE12: Joystick ABAJO
   *         PB13: Boton USER
   *         PE14: Joystick IZDA
   *         PE15: Joystick CENTER
   */
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    //CODE GENERATION
    switch(GPIO_Pin)
    {
      case GPIO_PIN_10:  //UP DIP15 PB10
        joy_tx_msg.button = 'U';//1
            break;

      case GPIO_PIN_11:  //RIGHT DIP16 PB11
        joy_tx_msg.button = 'R'; //2
            break;

      case GPIO_PIN_12:  //DOWN DIP12 PE12
        joy_tx_msg.button = 'D'; //3
            break;

      case GPIO_PIN_14:  //LEFT DIP13 PE14
        joy_tx_msg.button = 'L'; //4
            break;

      case GPIO_PIN_15:  //CENTER DIP14 PE15
        joy_tx_msg.button = 'C';//5
            break;
    }
    osMessageQueuePut(id_joy_MsgQueue, &joy_tx_msg, 0U, 0U);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
