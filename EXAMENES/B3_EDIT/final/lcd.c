#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"
#include "Arial12x12.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_lcd;                        // thread id
osMessageQueueId_t msg_lcd;
 
void ThLCD (void *argument);                   // thread function

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

static TIM_HandleTypeDef htim7;
unsigned char buffer[512];

static  uint16_t positionL1 = 0;//Pagina 0 
static  uint16_t positionL2 = 256;
  
int Init_lcd (void) {
 
  msg_lcd = osMessageQueueNew(16, sizeof(MSG_LCD),NULL);
  tid_lcd = osThreadNew(ThLCD, NULL, NULL);
  if (tid_lcd == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLCD (void *argument) {
  LCD_reset();
  LCD_Init();
  borrar_todo_lcd();
  LCD_update();
  
  MSG_LCD lcd;
  
  while (1) {
    osMessageQueueGet(msg_lcd, &lcd, NULL, osWaitForever);
    escribir_cadena(lcd.cadena, lcd.linea);
  }
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
  SPIdrv->Initialize(mySPI_callback);
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
  //TIENE RELOJ 42MHz
  __HAL_RCC_TIM7_CLK_ENABLE();
  htim7.Instance = TIM7; //APB1
  htim7.Init.Prescaler = 41;//1 us por cuenta
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
  
 // Seleccionar CS = 0;//Elige que esclavo va usar o que comunicacion SPI usa
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
 // Seleccionar A0 = 1;//Para escribir dato
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_SET);

 // Escribir un dato (data) usando la funci n SPIDrv->Send( );
  status = SPIdrv->Send(&data, sizeof(data));
  
  osThreadFlagsWait(0x80,osFlagsWaitAll,osWaitForever);
  
 // Seleccionar CS = 1;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);

}

void LCD_wr_cmd(unsigned char cmd){ // Funci n que escribe un comando en el LCD.

  uint32_t status;
 // Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
 // Seleccionar A0 = 0;
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13,GPIO_PIN_RESET);

 // Escribir un comando (cmd) usando la funci n SPIDrv->Send( );
  status = SPIdrv->Send(&cmd, sizeof(cmd));
  
  osThreadFlagsWait(0x80,osFlagsWaitAll,osWaitForever);
  
 // Seleccionar CS = 1;
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
}

void mySPI_callback(uint32_t event){
  uint32_t mask;
  mask = ARM_SPI_EVENT_TRANSFER_COMPLETE | 
  ARM_SPI_EVENT_DATA_LOST |
  ARM_SPI_EVENT_MODE_FAULT;
  
  if(event & mask){//success
    osThreadFlagsSet(tid_lcd, 0x80);
  }
}
