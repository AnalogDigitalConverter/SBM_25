/*P4-2  ADC 23-NOV-24*/
#define MAX_CHAR_LINE 32
#define LCD_BUFFER_SIZE 512

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"

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

  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
  
  UNUSED(TickPriority);

  return HAL_OK;
}
#endif

/* Public functions*/
extern ARM_DRIVER_SPI Driver_SPI1;

/* Public types*/
ARM_DRIVER_SPI* SPIdrv;
TIM_HandleTypeDef htim7;
ARM_SPI_STATUS SPI_Status;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static unsigned char buffer[LCD_BUFFER_SIZE];
static uint16_t positionL1 = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

static void delay(uint32_t n_microsegundos);

static void LCD_reset(void);
static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);
static void LCD_init(void);
static void LCD_update(void);
static void symbolToLocalBuffer_L1(uint8_t symbol);
static void buffer_update(const char string[MAX_CHAR_LINE]);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Custom SW-based delay function apart from HAL
  * @param  n_microsegundos: Desired delay duration in microseconds
  * @retval None
  */
static void delay(uint32_t n_microsegundos){

    /*TIM7(internal only) runs out of APB1*/
    __HAL_RCC_TIM7_CLK_ENABLE();
    htim7.Instance = TIM7;

    /*Current Setting 	HCLK = 168MHz
                        APB2Pre=HCLK_DIV2->PCLK2->84MHz
                        APBTimers'x2'->168MHz*/
    /*Set Prescaler to PCLK2(MHz) - 1 = (84-1) MHz*/
    htim7.Init.Prescaler = 83;
    htim7.Init.Period = n_microsegundos - 1;

    HAL_TIM_Base_Init(&htim7);
    HAL_TIM_Base_Start(&htim7);

    /*Synthetic Lock, waiting for timer End Of Count flag*/
    while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)){
        /*Synthetic Lock, waiting for timer End Of Count flag*/
    }
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

    HAL_TIM_Base_Stop(&htim7);
    HAL_TIM_Base_DeInit(&htim7);

    __HAL_RCC_TIM7_CLK_DISABLE();
}

static void LCD_reset(void){
    static GPIO_InitTypeDef GPIO_InitStruct_LCD;

    GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
    GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;

    /*CS*/
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct_LCD.Pin = GPIO_PIN_14;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_LCD);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

    /*A0*/
    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitStruct_LCD.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_LCD);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

    /*nReset*/
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct_LCD.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_LCD);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);


    /*SPI - HW init Sequence*/
    SPIdrv-> Initialize(NULL);
    SPIdrv-> PowerControl(ARM_POWER_FULL);
    SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);

    /* Reset minimum pulse width @3.3V = 1.0 microseconds*/
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
    delay(1);

    /* Time to Reset completion from valid reset @3.3V = 1.0 ms*/
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    delay(1000);

}

static void LCD_wr_data(unsigned char data){

    /*SPI1 Selected*/
    /*static ARM_SPI_STATUS SPI_Status; Publicly Created*/
    SPIdrv = &Driver_SPI1;

    /*CS = 0; A0 = 1(data)*/
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

    /*SPI Communication*/
    SPIdrv->Send(&data, sizeof(data));
    do{
        SPI_Status = SPIdrv->GetStatus();
    }while(SPI_Status.busy);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_wr_cmd(unsigned char cmd){

    /*SPI1 Selected*/
    /*static ARM_SPI_STATUS SPI_Status; Publicly Created*/
    SPIdrv = &Driver_SPI1;

    /*CS = 0; A0 = 0(cmd)*/
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);

    /*SPI Communication*/
    SPIdrv->Send(&cmd, sizeof(cmd));
    do{
        SPI_Status = SPIdrv->GetStatus();
    }	while(SPI_Status.busy);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_init(void){
    /*Display off*/
    LCD_wr_cmd(0XAE);
    /*Vpolarity = 1/9*/
    LCD_wr_cmd(0xA2);
    /*Normal RAM Adressing*/
    LCD_wr_cmd(0xA0);
    /*Normal COM Out Scan*/
    LCD_wr_cmd(0xC8);
    /*Rinternal = 2*/
    LCD_wr_cmd(0x22);
    /*PWR On*/
    LCD_wr_cmd(0x2F);
    /* Start Line = 0*/
    LCD_wr_cmd(0x40);
    /*Display On*/
    LCD_wr_cmd(0xAF);
    /*Contrast enable*/
    LCD_wr_cmd(0x81);
    /*Contrast Setting Value*/
    LCD_wr_cmd(0x17);
    /*Normal Display setting*/
    LCD_wr_cmd(0xA4);
    /*Normal LCD Display*/
    LCD_wr_cmd(0xA6);
}

static void LCD_update(void){
    int i;
    LCD_wr_cmd(0x00);	/*Set 0 to 4 LSB of Address*/
    LCD_wr_cmd(0x10);	/*Set 0 to 4 MSB of Address*/

    LCD_wr_cmd(0xB0);	/*Page 0 Address*/
    for(i = 0; i < 128; i++){LCD_wr_data(buffer[i]);}

    LCD_wr_cmd(0x00);	/*Set 0 to 4 LSB of Address*/
    LCD_wr_cmd(0x10);	/*Set 0 to 4 MSB of Address*/

    LCD_wr_cmd(0xB1);	/*Page 1 Address*/
    for(i = 128; i < 256; i++){LCD_wr_data(buffer[i]);}

    LCD_wr_cmd(0x00);	/*Set 0 to 4 LSB of Address*/
    LCD_wr_cmd(0x10);	/*Set 0 to 4 MSB of Address*/

    LCD_wr_cmd(0xB2);	/*Page 2 Address*/
    for(i = 256; i < 384; i++){LCD_wr_data(buffer[i]);}

    LCD_wr_cmd(0x00);	/*Set 0 to 4 LSB of Address*/
    LCD_wr_cmd(0x10);	/*Set 0 to 4 MSB of Address*/

    LCD_wr_cmd(0xB3);	/*Page 3 Address*/
    for(i = 384; i < 512; i++){LCD_wr_data(buffer[i]);}
}

static void symbolToLocalBuffer_L1(uint8_t symbol){

    uint8_t i, value1, value2;
    uint16_t offset = 0;

    /*Character selector conversion for font array finder*/
    offset = 25 * (symbol - ' ');

    /*Character polarity acquisition loop for all 12x12 pixels*/
    for(i = 0; i<12; i++){

      /*Pixel Value Aquisition from font*/
      value1 = Arial12x12 [offset + i*2 + 1];
      value2 = Arial12x12 [offset + i*2 + 2];

      /*Buffer population from selected character*/
      buffer [i + positionL1] = value1;
      buffer [i + 128 + positionL1] = value2;

    }
    positionL1 = positionL1 + Arial12x12 [offset];
}


/**
 * @brief  Auxiliar function for populating LCD buffer with Arial12x12 character
 * @param  symbol: ASCII value of a given 'character'
 * @retval None
 */
static void LCD_text_update(const char string[MAX_CHAR_LINE]){

  static int i = 0;

  /*Buffer clear if exixst previous text (positionL1 greater than 0)*/
  for(i = 0; i < positionL1; i++){
    buffer[i] = 0x00;
    buffer[i + 128] = 0x00;
  }

  /*Buffer update loop for every character in string*/
  for(i = 0; i < strlen(string); i++){
    symbolToLocalBuffer_L1(string[i]);
  }

  /*If buffer not fully populated, populate with 0 to avoid remaining old text
   *not overwritten due to inferior length*/
  for(i = positionL1; i < 128; i++){
    buffer[i] = 0x00;
    buffer[i + 128] = 0x00;
  }

  /* LCD_update integration, LCD_buffer_update will allways be called
   * upon updating the display */
  LCD_update();
}



/*MAIN************************************************************************/

int main(void)
{

  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here*/
  LCD_reset();
  LCD_init();

  //P4_2
  /*LCD in Operation, repeat each frame/text string*/
  LCD_text_update("Prueba de texto L1");


#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */

  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1){

  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1 -> SYSCLK == HCLK
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

/*****************************************************************************/

static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
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
