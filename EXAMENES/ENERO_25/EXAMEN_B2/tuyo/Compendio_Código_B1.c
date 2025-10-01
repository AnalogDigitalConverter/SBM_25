#include "stm32f4xx_hal.h"

//LED1 PB0
//LED2 PB7
//LED3 PB14


// INICIAR LEDS
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


// USAR LEDS
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  
/*****************************************************************************/
Fsys(168MHz) -> APB1 ->(84Mhz)->/Init.Prescaler->TIM7 Counter (Init.Period)
Valor de contador comparador de autorecarga que promptea Interrupción

1/84M = 780uS

/*****************************************************************************/
userbutton
/*****************************************************************************/
__HAL_RCC_GPIOC_CLK_ENABLE();

GPIO_InitStruct.Pin = GPIO_PIN_13;
GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
GPIO_InitStruct.Pull = GPIO_PULLDOWN;
HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);



en xx_it.c
//HANDLER SIEMPRE IGUAL. CONTROLA LA FLAG DE LA INTERRUPCIÓN A TRAVÉS DE CAPA HAL. PROMPTEA EL CALLBACK.
  void EXTI15_10_IRQHandler (void) {
	HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
}
//LÓGICA DE CAMBIOS!!
void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin){
if (GPIO_Pin == GPIO_PIN_13){
		if (t_delay == 125)
			t_delay = 500;
		else 
			t_delay = (t_delay)/2;
		}
}

/*****************************************************************************/
/*timer 7 y systemcoreclock*/
	/*Timer 7 Interrupt code*/
	htim7.Instance = TIM7;					//Adjudicación de Handler a TIM7
	htim7.Init.Prescaler = 41999;			//84MHz/42k = 2kHz
	/*htim7.Init.Period = 999;				//EJ.A = 500 ms		*/
	htim7.Init.Period = 2999;				//Autoreload Register [2999 to 0] == 3000/(2000/s) = 1.5s

		//HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM7_IRQn); 	//Enable IRQ
	__HAL_RCC_TIM7_CLK_ENABLE();		//Enable TIM7 Clock Source

	HAL_TIM_Base_Init(&htim7);			//TIM Config Init
	HAL_TIM_Base_Start_IT(&htim7);	//TIM1 Init
	
	
	  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  
/*****************************************************************************/
--DERECHA
static void init_Joystick_mbed(void){

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15 ;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	}