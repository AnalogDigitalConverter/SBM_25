#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"
#include "Arial12x12.h"


osTimerId_t id_refresh_tmr;
void refresh_Callback(void *args);

osTimerId_t id_lock_tmr;
void lock_Callback(void *args);

/*----------------------------------------------------------------------------
 *      Thread 'lcd'
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_lcd;                        // thread id
osMessageQueueId_t msg_lcd;
 
void ThLCD (void *argument);                   // thread function

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

static TIM_HandleTypeDef htim7;
unsigned char buffer[512];

static uint16_t positionL1 = 0;//Pagina 0 
static uint16_t positionL2 = 256;
static uint8_t 	col = 0;

static void pinta_linea(int columna);
static void pinta_bloque(int fila_init, int col_init, int alto, int ancho);
static void pinta_diagonal(int columna);

static void ST_PC13_UserButton_Init(void);
  
int Init_lcd (void) {
 
  msg_lcd = osMessageQueueNew(16, sizeof(MSG_LCD),NULL);
  tid_lcd = osThreadNew(ThLCD, NULL, NULL);
  if (tid_lcd == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLCD (void *argument) {
	
  id_refresh_tmr = osTimerNew(refresh_Callback, osTimerPeriodic, NULL, NULL);
	
  LCD_reset();
  LCD_Init();
  borrar_todo_lcd();
	
	pinta_bloque(4,24,4,4);
	pinta_diagonal(70);
	LCD_update();
	osDelay(5000u);
	borrar_todo_lcd();
  
  if (!osTimerIsRunning(id_refresh_tmr))
	{
	  osTimerStart(id_refresh_tmr, 200U);
	}
  
	ST_PC13_UserButton_Init();
  
  while (1) {
		osThreadFlagsWait(EXTI_JOY_IRQ_FLAG,osFlagsWaitAny,osWaitForever);
		osTimerStart(id_lock_tmr, 5000U);
		borrar_todo_lcd();
		col = 0;
  }
}

void refresh_Callback(void *args)
{
	if (col<64){
		pinta_linea(63-col);
		pinta_linea(64+col);
		pinta_linea(256+63-col);
		pinta_linea(256+64+col);
		col++;
		LCD_update();
	}
	
	//else{
	//	while(1){/*LOCK*/}
	//}
}
void lock_Callback (void *args)
{
	
}

void EXTI15_10_IRQHandler (void){
	HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
		if (GPIO_Pin == GPIO_PIN_13)
			osThreadFlagsSet(tid_lcd, EXTI_JOY_IRQ_FLAG);
}

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


/*----------------------------------------------------------------------------
 *      FUNCIONES
 *---------------------------------------------------------------------------*/
 
void borrar_todo_lcd(void){
  int i = 0;

  for (i = 0; i <512 ; i++){
    buffer[i]= 0x00;
  }
  
  LCD_update();
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
static void pinta_pixel(int fila, int columna){ 
	
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
	uint8_t ancho = 8;
  for(i=0;i<ancho;i++){
    value1 = 0x01 << i;
    buffer[i+0+columna] = value1; // Pagina 0
  }
}

static void pinta_bloque(int fila_init, int col_init, int alto, int ancho)
{
	uint8_t i,j, value1, value_int, mask = 0;
	for (j = 0; j < alto; j++)
	{
		mask |= (1u << j); // Set the i-th bit
  }
	value1 ^= (0xFF & mask) << fila_init;
		
	for(i=0;i<ancho;i++){
		value1 = 0xFF << fila_init;
		buffer[i+0+col_init] = value1; // Pagina 0
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
void symbolToLocalBuffer_L1(uint8_t symbol){//caracter assci
  uint8_t i, value1, value2;
  uint16_t offset=0;
  //static  uint16_t positionL1 = 0;//importante static
  //positionL1 = 0;
  
  offset=25*(symbol - ' ');
  
  for (i = 0; i<12; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    
    buffer[i+positionL1]=value1;
    buffer[i+128+positionL1]=value2;
  }
  positionL1=positionL1+Arial12x12[offset];
}

void symbolToLocalBuffer_L2(uint8_t symbol){//caracter assci
  uint8_t i, value1, value2;
  uint16_t offset=0;
  //positionL2 = 0;//importante static
  
  offset=25*(symbol - ' ');
  
  for (i = 0; i<12; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    
    buffer[i+256+positionL2]=value1;
    buffer[i+384+positionL2]=value2;
  }
  positionL2=positionL2+Arial12x12[offset];
}

void LCD_Init(void){
  LCD_wr_cmd(0xAE); //Display off (1010(A) 1110(E))
  LCD_wr_cmd(0xA2); //Fija el valor de la relaci n de la tensi n de polarizaci n del LCD a 1/9
  LCD_wr_cmd(0xA0); //El direccionamiento de la RAM de datos del display es la normal
  LCD_wr_cmd(0xC8); //El scan en las salidas COM es el normal
  LCD_wr_cmd(0x22); //Fija la relaci n de resistencias interna a 2
  LCD_wr_cmd(0x2F); //Power on
  
  LCD_wr_cmd(0x40); //Display empieza en la l nea 0
  LCD_wr_cmd(0xAF); //Display ON
  LCD_wr_cmd(0x81); //Contraste
  LCD_wr_cmd(0x15); //Valor Contraste 
  LCD_wr_cmd(0xA4); //Display all points normal -- Pantalla reversa 0xA7
  LCD_wr_cmd(0xA6); //LCD Display normal
}

void LCD_update_L1(void){ //actualiza todas las p ginas
  int i;
  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
  LCD_wr_cmd(0xB0); // P gina 0

  for(i=0;i<128;i++){
  LCD_wr_data(buffer[i]);
  }

  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
  LCD_wr_cmd(0xB1); // P gina 1

  for(i=128;i<256;i++){
  LCD_wr_data(buffer[i]);
  }
}

void LCD_update_L2(void){ //actualiza todas las p ginas
  int i;
  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
  LCD_wr_cmd(0xB0); // P gina 0

  for(i=0;i<128;i++){
  LCD_wr_data(buffer[i]);
  }

  LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
  LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
  LCD_wr_cmd(0xB1); // P gina 1

  for(i=128;i<256;i++){
  LCD_wr_data(buffer[i]);
  }
}

void LCD_update(void){ //actualiza todas las p ginas
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
 LCD_wr_cmd(0xB0); // P gina 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la direcci n a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la direcci n a 0
 LCD_wr_cmd(0xB1); // P gina 1

 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Pagina 2
 
 for(i=256;i<384;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3

 for(i=384;i<512;i++){
 LCD_wr_data(buffer[i]);
 }
}

void LCD_reset(void){
  
  GPIO_InitTypeDef GPIO_InitStruct;
  
  //INICIALIZACI N Y CONFIGURACI N DRIVER SPI PARA LCD
  SPIdrv->Initialize(NULL);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8) , 20000000);

  //INICIALIZAR LOS PINES NECESARIOS (EL MOSI Y SCK YA ESTAN EN RTE_Device)
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  
  //GPIO_InitStruct.Pin = GPIO_PIN_5;//LCD_SCK
  //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  //GPIO_InitStruct.Pin = GPIO_PIN_7;//LCD_MOSI
  //HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_6;//LCD_RESET
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_13;//LCD_A0
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_14;//LCD_CS_N
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
  delay(1);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
  delay(1000);
}

void delay(uint32_t n_microsegundos){
  //TIENE RELOJ 84MHz
  __HAL_RCC_TIM7_CLK_ENABLE();
  htim7.Instance = TIM7; //APB1
  htim7.Init.Prescaler = 83;//1 us por cuenta
  htim7.Init.Period = (n_microsegundos - 1);
  
  HAL_TIM_Base_Init(&htim7); //Configurar timer
  HAL_TIM_Base_Start(&htim7); //Start timer basico

  while(!__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE)){
  }
  
  __HAL_TIM_CLEAR_FLAG(&htim7,TIM_FLAG_UPDATE);
  
  HAL_TIM_Base_Stop(&htim7);
  HAL_TIM_Base_DeInit(&htim7);
}

void LCD_wr_data(unsigned char data){ // Funci n que escribe un dato en el LCD.
  
  uint32_t status;
	static ARM_SPI_STATUS estado;
  
 // Seleccionar CS = 0;//Elige que esclavo va usar o que comunicacion SPI usa
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
 // Seleccionar A0 = 1;//Para escribir dato
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_SET);

 // Escribir un dato (data) usando la funci n SPIDrv->Send( );
  status = SPIdrv->Send(&data, sizeof(data));
  
    do{
    estado = SPIdrv->GetStatus();
  }while(estado.busy);
  
 // Seleccionar CS = 1;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);

}

void LCD_wr_cmd(unsigned char cmd){ // Funci n que escribe un comando en el LCD.

  uint32_t status;
	static ARM_SPI_STATUS estado;
 // Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
 // Seleccionar A0 = 0;
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_RESET);

 // Escribir un comando (cmd) usando la funci n SPIDrv->Send( );
  status = SPIdrv->Send(&cmd, sizeof(cmd));
  
	do{
	estado = SPIdrv->GetStatus();
	}while(estado.busy);
  
 // Seleccionar CS = 1;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
}

