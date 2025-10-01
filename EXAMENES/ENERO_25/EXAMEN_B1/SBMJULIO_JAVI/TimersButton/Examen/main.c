#include "main.h"

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#ifdef RTE_CMSIS_RTOS2_RTX5
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;
  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }
  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}
#endif

static void SystemClock_Config(void);
static void Error_Handler(void);

static void InitButton(void);
static void InitTimers(void);
static void iniciar_pines(void);

TIM_HandleTypeDef tim1;
TIM_HandleTypeDef tim2;
TIM_HandleTypeDef tim3;
TIM_HandleTypeDef tim4;
TIM_HandleTypeDef tim5;
TIM_HandleTypeDef tim6;
TIM_HandleTypeDef tim7;
TIM_HandleTypeDef tim8;
TIM_HandleTypeDef tim12;
TIM_HandleTypeDef tim13;
TIM_HandleTypeDef tim14;

int main(void){
  HAL_Init();
  SystemClock_Config();
  SystemCoreClockUpdate();
  
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  HAL_NVIC_EnableIRQ(TIM5_IRQn);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);

  /* App */
    InitTimers();
    InitButton();
    iniciar_pines();
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);



#ifdef RTE_CMSIS_RTOS2
  osKernelInitialize ();

  /* Init Threads */
  
  
  
  /* Init Threads */

  osKernelStart();
#endif

  while(1){
  }
}

static void SystemClock_Config(void){
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK){
    Error_Handler();
  }
  if (HAL_GetREVID() == 0x1001){
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

static void Error_Handler(void){
  while(1){}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line){
  while (1){}
}

#endif


//TIMER 1 Y 8 APB2
//TIMER 2, 3, 4, 5, 6, 7, 12, 13, 14 APB1
void InitTimers(void) {
  
  //timer2 (funciona!!)
  __HAL_RCC_TIM2_CLK_ENABLE();
  tim2.Instance = TIM2;
  tim2.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
  tim2.Init.Period = 1999;//2000Hz/2000=1Hz
  
  tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  tim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

  HAL_TIM_Base_Init(&tim2);
  HAL_TIM_Base_Start_IT(&tim2);
  
//  //timer3 (funciona!!)
//  __HAL_RCC_TIM3_CLK_ENABLE();
//  tim3.Instance = TIM3;
//  tim3.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
//  tim3.Init.Period = 1999;//2000Hz/2000=1Hz
//  
//  tim3.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  HAL_TIM_Base_Init(&tim3);
//  HAL_TIM_Base_Start_IT(&tim3);
//  
  //timer4 (funciona!!)
//    __HAL_RCC_TIM4_CLK_ENABLE();
//  tim4.Instance = TIM4;
//  tim4.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
//  tim4.Init.Period = 1999;//2000Hz/2000=1Hz
//  
//  tim4.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  HAL_TIM_Base_Init(&tim4);
//  HAL_TIM_Base_Start_IT(&tim4);
//  
//  //timer5 (funciona!!)
//  __HAL_RCC_TIM5_CLK_ENABLE();
//    
//  tim5.Instance = TIM5;
//  tim5.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
//  tim5.Init.Period = 1999;//2000Hz/2000=1Hz
//  
//  tim5.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  HAL_TIM_Base_Init(&tim5);
//  HAL_TIM_Base_Start_IT(&tim5);
//  
//  __HAL_RCC_TIM6_CLK_ENABLE();
//  tim6.Instance = TIM6;
//  tim6.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
//  tim6.Init.Period = 1999;//2000Hz/2000=1Hz
//  
//  tim6.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  HAL_TIM_Base_Init(&tim6);
//  HAL_TIM_Base_Start_IT(&tim6);
//  
//  //timer7 (funciona!!)
//  __HAL_RCC_TIM7_CLK_ENABLE();

//  tim7.Instance = TIM7;
//  tim7.Init.Prescaler = 41999;   //84Mhz/42000=2000Hz (max 65535)
//  tim7.Init.Period = 1999;//2000Hz/2000=1Hz
//  
//  tim7.Init.CounterMode = TIM_COUNTERMODE_UP;
//  tim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

//  HAL_TIM_Base_Init(&tim7);
//  HAL_TIM_Base_Start_IT(&tim7);
//  
//  //APB2//
//  

//  __HAL_RCC_TIM8_CLK_ENABLE(); //APB2
//  
//  //__HAL_RCC_TIM9_CLK_ENABLE();
//  
//  __HAL_RCC_TIM10_CLK_ENABLE(); //APB2
//  
//  //__HAL_RCC_TIM11_CLK_ENABLE();
//  
//  __HAL_RCC_TIM12_CLK_ENABLE();
//  
//  __HAL_RCC_TIM13_CLK_ENABLE();
//  
//  __HAL_RCC_TIM14_CLK_ENABLE();
}

void InitButton(){
  /* Boton AZUL*/ 
  static GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 
}
void iniciar_pines(void){
  static GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  /* Leds */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14; //0 verde, 7 azul, 14 rojo
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}


