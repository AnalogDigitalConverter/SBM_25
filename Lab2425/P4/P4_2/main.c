/*ADC 14-NOV-24*/
/*P4_1*/
/*ERROR DE SCT SE SOLVENTA CON PACKS -> DEVICE -> STARTUP*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"
#include "string.h"
#include <stdio.h>

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
unsigned char buffer[512];
static uint32_t positionL1, positionL2 = 0;
int i = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

//CUSTOM PRIVATE FUNCTIONS
/*Crear funcion de inicializacion de hw TIM7_INIT independiente*/
/*Delay solo debería inicializar el timer, hacer la espera y detenerlo*/
static void delay(uint32_t n_microsegundos);

static int SPI_init(void);

static void LCD_reset(void);
static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);

static void LCD_init(void);
static void LCD_update(void);
static void LCD_Clean(void);

//P4_1
static void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
static void symbolToLocalBuffer_L1(uint8_t symbol);
static void symbolToLocalBuffer_L2(uint8_t symbol);


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
  LCD_init();
  LCD_Clean();
  symbolToLocalBuffer_L1('H');
  LCD_update();


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

void delay(uint32_t n_microsegundos){

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
  
  __HAL_RCC_TIM7_CLK_DISABLE();
}

void LCD_reset(void){

  //CRITERIO DE LA PRACTICA: integrar inicializacion SPI en funcion reset
  if(!SPI_init()){

  /*RESET PROPIAMENTE DICHO*/
  //td>=1us, tclear>=1ms;
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

  delay(1000);
  }
}

int SPI_init(void)
{
  /**************CHECK RTE_device.h FOR SPI CONFIG ASSIGNMENTS************/
	static GPIO_InitTypeDef GPIO_InitStruct_LCD;
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
	
	/*SPI*/
	SPIdrv->Initialize(NULL);
	SPIdrv-> PowerControl(ARM_POWER_FULL);
	SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);

  return 0;
}

/*---------------------------------------------------
 *      			Escritura y comandos del LCD
 *---------------------------------------------------*/
/*ACCORDING TO SPI LCD DRIVER COMMAND TABLE, DATA & COMMAND WRITE INSTRUCTIONS*/
void LCD_wr_data(unsigned char data)
{
  static ARM_SPI_STATUS stat;

  //CS = 0
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  //A0 = 1
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

  //Send referenced data through SPI & BLOCK until SPI_BUS freed
  SPIdrv->Send(&data, sizeof(data));
  do{
    stat = SPIdrv->GetStatus();
  }while(stat.busy);

  //CS = 1
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char cmd)
{
  static ARM_SPI_STATUS stat;

  //CS = 0
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  //A0 = 0
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);

  //Send referenced data through SPI & BLOCK until SPI_BUS freed
  SPIdrv->Send(&cmd, sizeof(cmd));
  do{
    stat = SPIdrv->GetStatus();
  }while(stat.busy);

  //CS = 1
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_init(void){
	LCD_wr_cmd(0xAE);//display off
	LCD_wr_cmd(0xA2);//Fija el valor de la tensión de polarización del LCD a 1/9
	LCD_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8);//El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22);//Fija la relación de resistencias interna a 2
	LCD_wr_cmd(0x2F);//Power on
	LCD_wr_cmd(0x40);//Display empieza en la línea 0
	LCD_wr_cmd(0xAF);//Display ON
	LCD_wr_cmd(0x81);//Contraste
	LCD_wr_cmd(0x13);//Valor de contraste
	LCD_wr_cmd(0xA4);//Display all points normal
	LCD_wr_cmd(0xA6);//LCD Display Normal 0xA7->Reverse
}

/*---------------------------------------------------
 *      			Update del LCD
 *---------------------------------------------------*/
static void LCD_update(void){
	int i;	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB0);
	for(i = 0; i < 128; i++){
			LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB1);
	for(i = 128; i < 256; i++){
			LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2);
	for(i = 256; i < 384; i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3);
	for(i = 384; i < 512; i++){
		LCD_wr_data(buffer[i]);
	}
}

/*---------------------------------------------------
 *      			Limpieza del LCD
 *---------------------------------------------------*/
static void LCD_Clean(void){
  for(int i=0; i<512; i++)
  {
    buffer[i]=0x00;
  }
  LCD_update();
}

static void symbolToLocalBuffer_L1(uint8_t symbol){

  uint8_t i, value1, value2;
  uint16_t offset = 0;

  //?
  offset = 25 * (symbol - ' ');

  //?
  for(i = 0; i<12; i++){
    value1 = Arial12x12 [offset + i*2 + 1];   //posicion 1
    value2 = Arial12x12 [offset + i*2 + 2];   //posicion a la dcha

    buffer [i + positionL1] = value1;      //posicion 1 en buffer superior
    buffer [i + 128 +positionL1] = value2; //posicion 2 en buffer inferior

  }
  positionL1 = positionL1 + Arial12x12 [offset];
}

/*---------------------------------------------------
 *    			  Metodo de escritura general
 *---------------------------------------------------*/
static void LCD_write(uint8_t line, char str[]){
	int i;
	for(i=0; i<strlen(str); i++){
		symbolToLocalBuffer(line, str[i]);
	}
	positionL1 = 0;
	positionL2 = 0;
	LCD_update();
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
* @}
*/
