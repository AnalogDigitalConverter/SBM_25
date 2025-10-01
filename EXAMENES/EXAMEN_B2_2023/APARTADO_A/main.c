/*CÓDIGO PANTALLAS SIN RTOS*/
/*24-JUN-25*/
/*COMPROBADO*/

#include "main.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"
#include <string.h>
#include <stdio.h>

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

static void SystemClock_Config(void);
static void Error_Handler(void);

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef htim7; 

static void LCD_Reset(void);
static void LCD_init(void);
static void LCD_update(void);
static void delay (uint32_t n_microsegundos);
static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);

void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void borrar_todo_lcd(void);
void escribir_cadena(char cadena[], int linea);
unsigned char buffer[512];
int i, positionL1, positionL2;

static void pinta_linea(int columna);
static void pinta_pixel(int columna, int fila);
static void pinta_diagonal(int columna);
static void pinta_horizontal(int columna);

/* Private functions ---------------------------------------------------------*/
int main(void)
{
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here */
  LCD_Reset();     //Reseteamos el LCD
  LCD_init();      //Lo inicializamos 
	borrar_todo_lcd();

	/*
	pinta_pixel(32,0);

	pinta_linea(63);
	pinta_linea(64);
	pinta_linea(319);
	pinta_linea(320);
	*/
	
	pinta_horizontal(0);
	pinta_diagonal(256);
	pinta_diagonal(392);
	
	LCD_update();
	
  /* Infinite loop */
  while (1)
  {
  }
}
/*********************************FIN DEL MAIN********************************/

void borrar_todo_lcd(void){
  int i = 0;

  for (i = 0; i <512 ; i++){
    buffer[i]= 0x00;
  }
  
  LCD_update();
	positionL1 = 0;
	positionL2 = 256;
}

void borrar_linea_lcd(uint16_t linea){
  int i = 0;
  int j = 0;
  
  if (linea == 1){
    j = 0;
  } else{
    j = 256;
  }
  for (i = 0; i <256 ; i++){
    buffer[i+j]= 0x00;
  }
  
  if (linea == 1) {
    positionL1 = 0;
  } else{
    positionL2 = 256;
  }
  
  LCD_update();
}

void escribir_cadena(char cadena[], int linea){
  int i=0;
  
  borrar_linea_lcd(linea);
 
  for (i=0;i<strlen(cadena); i++){
    symbolToLocalBuffer(linea,cadena[i]);
  }

  LCD_update();
}



static void LCD_Reset(void){
	
	static GPIO_InitTypeDef GPIO_InitStruct;
	
	SPIdrv->Initialize(NULL);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;         //Lo ponemos en output digital
	GPIO_InitStruct.Pull = GPIO_PULLUP;                  //Lo ponemos en pull-up
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;        //Lo ponemos a alta velocidad
	GPIO_InitStruct.Pin = GPIO_PIN_13;                   //F13 A0
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;          //Lo ponemos en output digital
	GPIO_InitStruct.Pull = GPIO_PULLUP;                  //Lo ponemos en pull-up
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;   //Lo ponemos a alta velocidad
	GPIO_InitStruct.Pin = GPIO_PIN_6;                    //A6 Reset
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;          //Lo ponemos en output digital
	GPIO_InitStruct.Pull = GPIO_PULLUP;                  //Lo ponemos en pull-up
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;        //Lo ponemos a alta velocidad
	GPIO_InitStruct.Pin = GPIO_PIN_14;                   //D14 Chip select
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  delay(1000);
  
}

void delay (uint32_t n_microsegundos){
  
  __HAL_RCC_TIM7_CLK_ENABLE();
	
    htim7.Instance = TIM7;                        //Hace referncia al Timer 7
	
    htim7.Init.Prescaler = 83;                    //Dividir la frecuencia del reloj para el contador del timer
                                                  //aunque en el SYSCLK ponga 168M, en el periferico tiene UN preescaler, 
                                                  //de /4 antes de llega al APB1 ( esto en el caso del TIM7)
	
    htim7.Init.Period = n_microsegundos -1;       //Configurar el fin de cuenta del contador
  
 
    HAL_TIM_Base_Init(&htim7);                    //Inicializar el timer
    HAL_TIM_Base_Start(&htim7);                   //Iniciar la generación de interrupciones de desbordamiento
  
    while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)){}
      
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Stop(&htim7);
    HAL_TIM_Base_DeInit(&htim7);
    __HAL_RCC_TIM7_CLK_DISABLE();
}
static void LCD_wr_data(unsigned char data){

  static ARM_SPI_STATUS estado;

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);

  SPIdrv->Send(&data, sizeof(data));

  do{
    estado = SPIdrv->GetStatus();
  }while(estado.busy);
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

}

static void LCD_wr_cmd(unsigned char cmd){
 static ARM_SPI_STATUS estado;

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	
  SPIdrv->Send(&cmd, sizeof(cmd));
	
  do{
    estado = SPIdrv->GetStatus();
  }while(estado.busy);
 
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);

}

static void LCD_init(void){

  LCD_wr_cmd(0xAE);  //Display OFF
  LCD_wr_cmd(0xA2);  //Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
  LCD_wr_cmd(0xA0);  //El direccionamiento de la RAM de datos del display es la normal
  LCD_wr_cmd(0xC8);  //El scan en las salidas COM es el normal
  LCD_wr_cmd(0x22);  //Fija la relación de resistencias interna a 2
  LCD_wr_cmd(0x2F);  //Power on
  LCD_wr_cmd(0x40);  //Display empieza en la línea 0
  LCD_wr_cmd(0xAF);  //Display ON
  LCD_wr_cmd(0x81);  //Contraste 
  LCD_wr_cmd(0x12);  //Valor Contraste (a eleccion del usuario)
  LCD_wr_cmd(0xA4);  //Display all points normal
  LCD_wr_cmd(0xA6);  //LCD Display normal
  
}

static void LCD_update(void){
	
  static int i;             //Variable para recorrer todo el array  

  LCD_wr_cmd(0x00);         //4 bits de la parte baja de la dirección a 0
  LCD_wr_cmd(0x10);         //4 bits de la parte alta de la dirección a 0
  LCD_wr_cmd(0xB0);         //Pagina 0

  for(i=0;i<128;i++){       //Recorremos toda esa pagina

    LCD_wr_data(buffer[i]);

  }

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB1);         //Pagina 1

  for(i=128;i<256;i++){

    LCD_wr_data(buffer[i]);
  }

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB2);        //Pagina 2

  for(i=256;i<384;i++){

    LCD_wr_data(buffer[i]);
  }

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB3);        //Pagina 3

  for(i=384;i<512;i++){

    LCD_wr_data(buffer[i]);
  }

}

void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset=0;
  //positionL1 = 0;//Pagina 0 
  //positionL2 = 256;//lo pongo desde donde empieza, Pagina 2
  uint16_t position = 0;
  
  if (line == 1){
    position = positionL1;
  }else{
    position = positionL2;
  }
  
  offset=25*(symbol - ' ');
  
  for (i = 0; i<12; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    
    buffer[i+position]=value1;
    buffer[i+128+position]=value2;
  }
  
  if (line == 1){
    positionL1=positionL1+Arial12x12[offset];
  }else{
    positionL2=positionL2+Arial12x12[offset];
  }
  
}


/*PINTA LÍNEA VERTICAL CON CURSOR EN UBICACIÓN COLUMNA, ABARCA LAS DOS PÁGINAS DEL BÚFER. PARA BÚFER INFERIOR, SUMAR 256*/
static void pinta_linea(int columna){ 
	
  uint8_t i, value1, value2;
	uint8_t ancho = 1;
  for(i=0;i<ancho;i++){
		
    value1 = 0xFF;
    value2 = 0xFF;
 
    buffer[i+0+columna] = value1; // Pagina 0
    buffer[i+128+columna] = value2; // Pagina 1
  }
}
/*PINTA PÍXEL CON CURSOR EN UBICACIÓN COLUMNA. FILAS 0-7 */
static void pinta_pixel(int columna, int fila){ 
	
  uint8_t i, value1;
	uint8_t ancho = 1;
	if(fila < 9)
		for(i=0;i<ancho;i++){
			value1 = 0x01 << fila;
			buffer[i+0+columna] = value1; // Pagina 0
		}
}

/*PINTA PÍXEL CON CURSOR EN UBICACIÓN COLUMNA. FILAS 1-8 */
static void pinta_horizontal(int columna){ 
	
  uint8_t i, value1;
	uint8_t ancho = 128;
  for(i=0;i<ancho;i++){
    value1 = 0x01;
    buffer[i+0+columna] = value1; // Pagina 0
  }
}
/*PINTA DIAGONAL CON CURSOR EN UBICACIÓN COLUMNA. UN SÓLO BÚFER */
static void pinta_diagonal(int columna)
{
  uint8_t i, value1;
	uint8_t ancho = 128;
  for(i=0;i<ancho;i++){
    value1 = 0x01 << i;
    buffer[i+0+columna] = value1; // Pagina 0
  }
}

/*LEGACY FUNCTIONS*/
void LCD_symbolToLocalBuffer_L1(uint8_t symbol){

  uint8_t i, value1, value2;
  uint16_t offset = 0;
	static int positionL1 = 0;
  
  offset = 25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
    
    buffer[i+positionL2] = value1; // Pagina 0
    buffer[i+128+positionL2] = value2; // Pagina 1
  }
  positionL1 += Arial12x12[offset];
}

void LCD_symbolToLocalBuffer_L2(uint8_t symbol){

  uint8_t i, value1, value2;
  uint16_t offset = 0;
	static int positionL2 = 0;
  
  offset = 25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
    
    buffer[i+256+positionL2] = value1; // Pagina 2
    buffer[i+384+positionL2] = value2; // Pagina 3
  }
  positionL2 += Arial12x12[offset];
}



/*****************************************************************************/
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
