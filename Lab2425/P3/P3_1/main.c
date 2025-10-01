
/*ADC 12-NOV-24*/

/*ERROR DE SCT SE SOLVENTA CON PACKS -> DEVICE -> STARTUP*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Driver_SPI.h"

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
  *    and return auxiliary tick counter value */
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

/** @addtogroup STM32F4xx_HAL_Examples
* @{
*/

/** @addtogroup Templates
* @{
*/

/****  SPI DRIVER IMPORT   ****/
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

TIM_HandleTypeDef htim7;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

//CUSTOM PRIVATE FUNCTIONS
/*Crear funcion de inicializacion de hw TIM7_INIT independiente*/
/*Delay solo debería inicializar el timer, hacer la espera y detenerlo*/
static void delay(uint32_t n_microsegundos);
static void SPI_init(void);
static void LCD_reset(void);


/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
  /* STM32F4xx HAL library initialization: */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
  */
  LCD_reset();


  #ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  *  Example: osThreadNew(app_main, NULL, NULL); */

  /* Start thread execution */
  osKernelStart();
  #endif

  /* Infinite loop */
  while (1)
  {
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
*            PLL_M                          = 25
*            PLL_N                          = 336
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
  *    clocked below the maximum system frequency, to update the voltage scaling value
  *    regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  *    clocks dividers */
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

/*---------------------------------------------------------------------------*/

static void delay(uint32_t n_microsegundos){

  /*Nota: 	cada vez que se instancia hay que reconfigurar period+base init*/
  __HAL_RCC_TIM7_CLK_ENABLE();

  //delay value pass-through
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 83;
  htim7.Init.Period = n_microsegundos - 1;

  HAL_TIM_Base_Init(&htim7);
  HAL_TIM_Base_Start(&htim7);

  //wait until end of count then clear
  while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE))
  {
    //Bloqueante
  }
  __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

  //As new timer may require different period, DeInit is required
  HAL_TIM_Base_Stop(&htim7);
  HAL_TIM_Base_DeInit(&htim7);
}

static void LCD_reset(void){

  //CRITERIO DE LA PRACTICA: integrar inicializacion SPI en funcion reset
  SPI_init();

  /*RESET PROPIAMENTE DICHO*/
  //td>=1us, tclear>=1ms;
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

  delay(1000);
}

static void SPI_init(void)
{
  /**************CHECK RTE_device.h FOR SPI CONFIG ASSIGNMENTS************/
  static GPIO_InitTypeDef GPIO_InitStruct_LCD;

  /*SPI HW INIT, EXTRAER DEL RESET*/
  /*SPI*/
  SPIdrv->Initialize(NULL);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);

  /*CS*/
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

  /*A0*/
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

  /*Reset*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_LCD);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  *    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
* @}
*/

/**
* @}
*/
