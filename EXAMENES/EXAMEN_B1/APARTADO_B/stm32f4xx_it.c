
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

#ifdef _RTE_
#include "RTE_Components.h"             /* Component selection */
#endif

/* Private variables ---------------------------------------------------------*/
static uint8_t cnt, cnt_user = 0;

/* Private function prototypes -----------------------------------------------*/
/* Public Types -------------------------------------------------------------*/
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim6;

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/


/**
  * @brief  This function strictly handles Basic TIM7 (APB1 45MHz x2 Max.Freq)
  *  		interrupt request.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim7);
}
void TIM6_DAC_IRQHandler(void){
	HAL_TIM_IRQHandler(&htim6);
}

void EXTI15_10_IRQHandler (void){
	HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
}

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Callbacks                         */
/******************************************************************************/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if(htim->Instance == TIM7)
	{
		cnt += 1;
		
		if(cnt >= 6 & cnt <= 12)
		{
			//LED3OFF
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
			//LEDS1/2 BLINK ->not
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7);
		}
		
		//OJO CON LAS CONDICIONES: Tiene sentido >= 12 pero no funcionaba bien. ASEGURA OFF
		else if(cnt > 12 & cnt < 18)
		{
			//LED1OFF
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			//LED2/3 BLINK ->not
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7 | GPIO_PIN_14);
			
		}
		else if(cnt >= 18)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_RESET);
		}
		
  }
	if(htim->Instance == TIM6)
	{
    //Control de rebotes
    HAL_TIM_Base_Stop(&htim6);
    
    //condición ejercicio, reconocer solo a partir de 3seg
	  if (cnt >= 6)
    {
      cnt_user++;
    }
    
    if (cnt_user == 5)
    {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_SET);
      
      while (1){}
    }
	
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /**CHULETA ***********************
  * if (GPIO_Pin == GPIO_PIN_?){
  *  //lógica de excepcion del botón
  * } 
  *********************************/
  
  //User Button Interrupt Logic
  if (GPIO_Pin == GPIO_PIN_13)
  {
		//arrancar timer rebotes	
		HAL_TIM_Base_Start_IT(&htim6);    //TIM6 Init
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
