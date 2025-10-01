/* P2_4 - ANTONIO DONA CANDELA - 5OCT24 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
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

/**
  * Override default HAL_InitTick function
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
  
  UNUSED(TickPriority);

  return HAL_OK;
}
#endif


/* Public variables ---------------------------------------------------------*/
//han de ser accedidos por stm32f4xx_it
//(son modificados por interrupciones/callbacks)
uint16_t periodo_tim2;
TIM_HandleTypeDef htim2, htim3, htim7;
TIM_OC_InitTypeDef TIM_Channel_InitStruct;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef GPIO_InitStruct;
static TIM_IC_InitTypeDef sConfigIC;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

//HW_INIT_Prototypes ---------------------------------------------------------*/
static void ST_LED_Init(void);
static void ST_PC13_UserButton_Init(void);
static void ST_PB11_Pin_Init(void);
static void TIM2_OC_Init(void);
static void TIM3_IC_Init(void);
static void TIM7_Init(void);


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
  */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here */
  ST_LED_Init();
  ST_PC13_UserButton_Init();
  ST_PB11_Pin_Init();
  
  TIM2_OC_Init();
  TIM3_IC_Init();
  TIM7_Init();
  
#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */

  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1)
  {
	/*Nothing to be seen, all independent IRQ HW requests*/
	/*Refer to stm32f4xx_it.c ****************************/
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 4
  *            PLL_N                          = 168
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
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
  RCC_OscInitStruct.PLL.PLLN = 168;
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

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/*---------------------------------------------------------------------------*/
/*----------------- Custom HW Initialization Functions ----------------------*/
/*---------------------------------------------------------------------------*/
/**
  * @brief  Configures and assigns LD1(Green - PB0), LD2(Blue - PB7),
  *  		and LD3(Red - PB14) present on NULCEO-144 Board
  * @param  None
  * @retval None
  */
static void ST_LED_Init(void){

  /*LD1 - Green(Pin0), LD2 - Blue(Pin7), LD3 - Red(Pin14)*/
  GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  /*CLK_ENABLE and Pin-Port Assign*/
  __HAL_RCC_GPIOB_CLK_ENABLE();
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  Configures the Interrupt Vector and assigns the User Button B1
  * 		(PC13 Pin) present on NULCEO-144 Board
  * @param  None
  * @retval None
  */
static void ST_PC13_UserButton_Init(void){

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

/**
  * @brief  Configures and Starts PB11 pin as TIM2 output as AF1.
  * @param  None
  * @retval None
  */
static void ST_PB11_Pin_Init(void){

  /*HAL Clock Channelling / Enable*/
  __HAL_RCC_GPIOB_CLK_ENABLE(); 	//INICIALIZADO DE RELOJ EN PUERTO B

  /*GPIO HW Config*/
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pin       = GPIO_PIN_11;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;

  /*GPIO HW Init*/
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  Configures mbed_Joystick inputs.
  * @param  None
  * @retval None
  */
static void mbed_Joystick_Init(void)
{
/*1. PB10 - mbed 15 = UP  */
/*2. PB11 - mbed 16 = R   */
/*3. PE12 - mbed 12 = DN  */
/*4. PE14 - mbed 13 = L   */
/*5. PE15 - mbed 14 = CTR */

/* En caso de querer optimizar las macros de inicializacion de reloj, generar*/
/* funcion de inicializacion de relojes de todos los puertos.                */
/* Corre el riesgo de olvidar inicializar reloj                              */

  /*HAL Clock Channelling / Enable*/
  __HAL_RCC_GPIOB_CLK_ENABLE(); 	//INICIALIZADO DE RELOJ EN PUERTO B
  /*GPIO HW Config*/
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  /*PB11 currently in use as Output. */
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
  /*GPIO HW Init*/
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*HAL Clock Channelling / Enable*/
  __HAL_RCC_GPIOE_CLK_ENABLE(); 	//INICIALIZADO DE RELOJ EN PUERTO E
  /*GPIO HW Config*/
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  
  /*GPIO HW Init*/
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}
/**
  * @brief  Configures and Starts TIM7 Interrupts at 1.5s intervals from 84MHz
  *  		bus timer clock (APB1 45MHz x2 Timer Clock)
  * @param  None
  * @retval None
  */
static void TIM7_Init(void){
  /*Declarado TypeDef timer Struct TypeDef
    GPIO_InitTypeDef GPIO_InitStruct;*/
  /*Declarado Handle
    TIM_HandleTypeDef htim7;*/

/* TIMER 7 CONFIG */
  __HAL_RCC_TIM7_CLK_ENABLE();      //Enable TIM7 Perif Clock Source
  
/* TIMER 7 NVIC IRQ Enable */
  HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);    //Enable IRQ
  
/* For SystemCoreClock = 168MHz, APB1_APB2_TIM_CLK=84MHz */
  htim7.Instance       = TIM7;
  
  //Up to 65535 factor, divide to 41999 (count 0 to 41999 for EOC)
  htim7.Init.Prescaler = 41999;     //84MHz/42k = 2kHz

  //Generate EOC pulse interrupt at 3000th count
  htim7.Init.Period    = 2999;      //[2999 to 0] == 3000/2000/s = 1.5s
  
  //Load to handler pointer
  HAL_TIM_Base_Init(&htim7);        //TIM Config
  
  //Start (can be placed whenever is required to start
  HAL_TIM_Base_Start_IT(&htim7);    //TIM1 Init
}

/**
  * @brief  SQUARE SIGNAL GENERATOR:
  *         Configures and Starts TIM2 in Output Compare Mode at 0.5ms
  *         intervals from 84MHz bus timer clock (APB1 45MHz x2 Timer Clock)
  *         and assigns it to PB11 pin through TIM_CHANNEL_4.
  * @param  None
  * @retval None
  */
static void TIM2_OC_Init(void){
  /*Declarado TypeDef timer Struct TypeDef
    GPIO_InitTypeDef GPIO_InitStruct;*/
  /*Declarado Handle
    TIM_HandleTypeDef htim2;*/

/* TIMER 7 CONFIG */
  __HAL_RCC_TIM2_CLK_ENABLE();        //Enable TIM2 Perif Clock Source
  
  /*Timer Channel HW Config*/
  //Comportamiento standard del timer (SQUARE SIGNAL)
  TIM_Channel_InitStruct.OCMode     = TIM_OCMODE_TOGGLE;
  //init a nivel alto
  TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
  TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
  
  /*For SystemCoreClock = 168MHz, APB1_APB2_TIM_CLK=84MHz */
  htim2.Instance       = TIM2;
  
  /*El código va a pedir el cambio de la de salida a una mayor frecuencia.   *
   *Es preferible actualizar el Period vs Prescaler, requiriendo un menor    *
   *valor de EOC. Envia interrupciones mas recurrentemente.                  */
   
  //Up to 65535 factor, divide to 41999 (count 0 to 41999 for EOC)
  htim2.Init.Prescaler = 1;             //84MHz/2 = 42M (probar 0)
  
  //Generate EOC pulse interrupt at ? count
  htim2.Init.Period    = 41999;         //[41999 to 0] == 42M/42k = 1kHz
  
  //Load to handler pointer (Output Compare Mode)
  HAL_TIM_OC_Init(&htim2);              //TIM Config
  HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4);
  
  //Start (can be placed whenever is required to start)
  /*Timer_2 CHA_4 Init*/
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);
}

/**
 * @brief  FREQ METER:
 *         Configures and Starts TIM3 in Input Capture Mode at 6kHz sampling
 *         freq from 84MHz bus timer clock (APB1 (45MHz max.) 42MHz x2
 *         Timer Clock) and assigns it to PB11 pin through TIM_CHANNEL_4.
 * @param  None
 * @retval None
 */
static void TIM3_IC_Init(void){
  /*Declarado TypeDef
  TIM_IC_InitTypeDef sConfigIC;*/
  /*Declarado Handle
   TIM_HandleTypeDef htim3;*/

  /*Interrupt & Clock Enable for Timer 3*/
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
  __HAL_RCC_TIM3_CLK_ENABLE();

  htim3.Instance = TIM3;

  htim3.Init.CounterMode    = TIM_COUNTERMODE_UP;

  /*Una alta frecuencia permite leer altas frecuencias de acuerdo con Nyquist*/
  /*Desafortunadamente, esto supone que el contador de periodo hace overflow */
  /*con demasiada asiduidad. Adaptando a bajas frecuencias de entrada.       */
  /*    -> 1kHz-2.5kHz -> 5kHz min                                           */
  /*    -> 6kHz -> Prescaler /14k [0-13999], max Period(overflow avoidance)  */
  /*Alta resolucion/frecuencia -> bajo Prescaler, max Period                 */

  //SI VIENE DEL APB1 -> 84MHz
  //htim3.Init.Prescaler      = 1;       //Div /2 -> 48M
  //htim3.Init.Period         = 47999;   //48M / 48k/Hz = 1kHz
  htim3.Init.Prescaler        = 13999;
  htim3.Init.Period           = 65535;

  htim3.Init.ClockDivision    = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim3);

  //HAL_TIM_IC_Init(&htim3);
  sConfigIC.ICPolarity  = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0;
  HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);

  /*Timer Start*/
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
}


