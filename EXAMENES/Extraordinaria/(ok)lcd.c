#include "cmsis_os2.h"                          // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 'lcd'                                                         *
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_LCD_Th;                        // thread id
 
void Thread (void *argument);                   // thread function
 
int Init_LCD_Th (void) {
 
  tid_LCD_Th = osThreadNew(LCD_Th, NULL, NULL);
  if (tid_LCD_Th == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void LCD_Th (void *argument) {
	
  LCD_reset();
  LCD_init();
  LCD_update();

  while (1) {
	/*Message capture*/
    osMessageQueueGet(LCD_MsgQueue, &msg_send, NULL, osWaitForever);
	
	/*Buffer clear and Buffer Write*/
	LCD_clear();
	LCD_write(1, msg_send.mensaje1);
	LCD_write(2, msg_send.mensaje2);
	
	//osDelay(100);
    osThreadYield();                            // suspend thread
  }
}

//Funcions
static void LCD_reset(void){

/*Considera sacar la configuracion de pines fuera de la instruccion de reset*/
/* e integrarlo en init */
  static GPIO_InitTypeDef GPIO_InitStruct_LCD;
  
  /*CS*/
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_LCD);
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  /*-------------------------------------------------*/
  
  /*A0*/
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_LCD);
  
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
  /*-------------------------------------------------*/

  
  /*Reset*/
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
  GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct_LCD.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_LCD);
  
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  /*-------------------------------------------------*/
  
  /*SPI*/
  SPIdrv->Initialize(NULL);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
  
  //Reset pulse: 1ms low
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  osDelay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  /*-------------------------------------------------*/
  
  osDelay(1000);
}

static void LCD_wr_data(unsigned char data){
  static ARM_SPI_STATUS stat;
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
  
  SPIdrv->Send(&data, sizeof(data));
  do{
    stat = SPIdrv->GetStatus();
  }while(stat.busy);
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_wr_cmd(unsigned char cmd){
  static ARM_SPI_STATUS stat;
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
  
  SPIdrv->Send(&cmd, sizeof(cmd));
  do{
    stat = SPIdrv->GetStatus();
  }	while(stat.busy);
  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_init(void){
  LCD_wr_cmd(0xAE);//display off
  LCD_wr_cmd(0xA2);//Fija el valor de la tensión de polarización del LCD a 1/9
  LCD_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
  LCD_wr_cmd(0xC8);//El scan en las salidas COM es el normal
  LCD_wr_cmd(0x22);//Fija la relación de resistencias interna a 2
  LCD_wr_cmd(0x2F);//Power on
  LCD_wr_cmd(0x40);//Display empieza en la línea 0
  LCD_wr_cmd(0xAF);//Display ON
  LCD_wr_cmd(0x81);//Contraste
  LCD_wr_cmd(0x17);//Valor de contraste
  LCD_wr_cmd(0xA4);//Display all points normal
  LCD_wr_cmd(0xA6);//LCD Display Normal // A6 MODO NORMAL A7 MODO INVERSO
}

static void LCD_update(void){
  int i;	
  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB0);
  for(i = 0; i < 128; i++){LCD_wr_data(buffer[i]);}

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB1);
  for(i = 128; i < 256; i++){LCD_wr_data(buffer[i]);}

  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB2);
  for(i = 256; i < 384; i++){LCD_wr_data(buffer[i]);}
  
  LCD_wr_cmd(0x00);
  LCD_wr_cmd(0x10);
  LCD_wr_cmd(0xB3);
  for(i = 384; i < 512; i++){LCD_wr_data(buffer[i]);}
}

void LCD_clear(void){
  for(i = 0; i<512; i++)
    buffer[i] = 0x00;
}

void LCD_write(uint8_t line, char a[]){
  static int n;
  for(n = 0; n < strlen(a); n++){
    symbolToLocalBuffer(line, a[n]);
  }
  LCD_update();

  /*Ubicacion de Cursor. 0 para enrasado a la izda inicial.*/
  positionL1 = 0;
  positionL2 = 0;
}

static void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
  if (line == 1){
    symbolToLocalBuffer_L1(symbol);}
  if (line == 2){
    symbolToLocalBuffer_L2(symbol);}
}

static void symbolToLocalBuffer_L1(uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset = 0;

  offset = 25*(symbol - ' ');

  for (i=0; i<12 ; i++)
  {    
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    buffer[i + positionL1] = value1;
    buffer[i + 128 + positionL1] = value2;
  }
  
  positionL1 = positionL1 + Arial12x12[offset];
}

static void symbolToLocalBuffer_L2(uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset = 0;

  offset = 25*(symbol - ' ');
  
  for (i=0; i<12 ; i++)
  {  
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    buffer[i + 256 + positionL2] = value1;
    buffer[i + 384 + positionL2] = value2;
  }
  
  positionL2 = positionL2 + Arial12x12[offset];
}


