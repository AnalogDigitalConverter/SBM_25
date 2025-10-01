
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

//HW_INIT
static void ST_LED_Init(void);
static void ST_PC13_UserButton_Init(void);

//TIM_INIT
//no static para poder extern en interrupciones.
GPIO_InitTypeDef GPIO_InitStruct;

static void TIM7_Init(void);
TIM_HandleTypeDef htim7;

static void TIM6_Init(void);
TIM_HandleTypeDef htim6;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  /* STM32F4xx HAL library initialization:*/
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

/* Add your application code here*/

  /*HW_INIT*/
	ST_LED_Init();
	ST_PC13_UserButton_Init();
  
  /*TIMER_INIT*/
	TIM7_Init();
	TIM6_Init();
  
  /* Infinite loop */
  while (1)
  {
  }
}

/*SystemCoreClock = 100MHz = PLLM = 4; PLLN = 100; .PLLP = RCC_PLLP_DIV2;*/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

/**
  * @brief  Configures and Starts TIM7 Interrupts at 1.5s intervals from 84MHz
  *  		bus timer clock (APB1 45MHz x2 Timer Clock, 100MHz SystemCoreClock)
  * @param  None
  * @retval None
  */
static void TIM7_Init(void){

	//GPIO_InitTypeDef GPIO_InitStruct;
	//static TIM_HandleTypeDef htim7;

/* TIMER 7 CONFIG */
  __HAL_RCC_TIM7_CLK_ENABLE();      //Enable TIM7 Perif Clock Source
  
/* TIMER 7 NVIC IRQ Enable */
  HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);    //Enable IRQ
  
/* For SystemCoreClock = 100MHz, APB1_APB2_TIM_CLK=50MHz */
  htim7.Instance       = TIM7;
  
  //Up to 65535 factor, divide to 49999 (count 0 to 49999 for EOC)
  htim7.Init.Prescaler = 49999;     //50MHz/50k = 1kHz

  //Generate EOC pulse interrupt at 500th count
  htim7.Init.Period    = 499;      //[2999 to 0] == 500/1000/s = 0.5s
  
  //Load to handler pointer
  HAL_TIM_Base_Init(&htim7);        //TIM Config
  
  //Start (can be placed whenever is required to start
  HAL_TIM_Base_Start_IT(&htim7);    //TIM1 Init

}

/**
  * @brief  Configures and Starts TIM7 Interrupts at 1.5s intervals from 84MHz
  *  		bus timer clock (APB1 45MHz x2 Timer Clock, 100MHz SystemCoreClock)
  * @param  None
  * @retval None
  */
static void TIM6_Init(void){

/* TIMER 6 CONFIG */
  __HAL_RCC_TIM6_CLK_ENABLE();      //Enable TIM6 Perif Clock Source
  
/* TIMER 6 NVIC IRQ Enable */
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  
/* For SystemCoreClock = 100MHz, APB1_APB2_TIM_CLK=50MHz */
  htim6.Instance       = TIM6;
  
  //Up to 65535 factor, divide to 49999 (count 0 to 49999 for EOC)
  htim6.Init.Prescaler = 49999;     //50MHz/50k = 1kHz

  //Generate EOC pulse interrupt at 500th count
  htim6.Init.Period    = 79;      //[79 to 0] == 80/1000/s = 0.08s = 80ms
  
  //Load to handler pointer
  HAL_TIM_Base_Init(&htim6);        //TIM Config

}

/**
  * @brief  Configures and assigns LD1(Green - PB0), LD2(Blue - PB7),
  *  		and LD3(Red - PB14) present on NULCEO-144 Board
  * @param  None
  * @retval None
  */
static void ST_LED_Init(void){
	
  GPIO_InitTypeDef GPIO_InitStruct;

  /*LD1 - Green(Pin0), LD2 - Blue(Pin7), LD3 - Red(Pin14)*/
  GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  /*CLK_ENABLE and Pin-Port Assign*/
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*ARRANQUE ENCENDIDO*/
	HAL_GPIO_WritePin(GPIOB,  GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14, GPIO_PIN_SET);
}

/**
  * @brief  Configures the Interrupt Vector and assigns the User Button B1
  * 		(PC13 Pin) present on NULCEO-144 Board
  * @param  None
  * @retval None
  */
static void ST_PC13_UserButton_Init(void){
	
  GPIO_InitTypeDef GPIO_InitStruct;

  //NVIC - Interrupt Vector
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  //User Button B1 - PC13
  GPIO_InitStruct.Pin  = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  
  //CLK_ENABLE and Pin-Port Assign
  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
