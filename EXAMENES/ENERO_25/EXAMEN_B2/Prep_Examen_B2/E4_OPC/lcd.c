
#include "LCD.h"

#include <stdio.h>
#include <string.h>
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"
#include "Arial12x12.h"


static void LCD_Reset(void);
static void LCD_init(void);
static void LCD_update(void);
static void dataToBuffer(char data_L1[32], char data_L2[32]);
   
static void delay (uint32_t n_microsegundos);
static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);

static void LCD_symbolToLocalBuffer_L1(uint8_t symbol);
static void LCD_symbolToLocalBuffer_L2(uint8_t symbol);
static void LCD_symbolToLocalBuffer (uint8_t line, uint8_t symbol);

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef htim7;

unsigned char buffer[512];

uint8_t positionL1=0, positionL2=0;
int i;

static osThreadId_t id_Th_lcd;
int init_Th_lcd(void);
static void Th_lcd(void *arg);

static int Init_MsgQueue_lcd(void);
static osMessageQueueId_t id_MsgQueue_lcd;
osMessageQueueId_t get_id_MsgQueue_lcd(void);


static int Init_MsgQueue_lcd(void){
	id_MsgQueue_lcd = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_LCD), NULL);
	if(id_MsgQueue_lcd == NULL)
		return(-1);
	
	return (0);
}

osMessageQueueId_t get_id_MsgQueue_lcd(void){
	return id_MsgQueue_lcd;
}
int init_Th_lcd(void){
  LCD_Reset();
	LCD_init();
	
	id_Th_lcd = osThreadNew(Th_lcd, NULL, NULL);
  
	if(id_Th_lcd == NULL)
		return(-1);
  
 
	return(Init_MsgQueue_lcd());
}

static void Th_lcd(void *argument){ // funcion del hilo
  MSGQUEUE_OBJ_LCD msg_lcd;
	osStatus_t status;
	
  while(1){
//    status = osMessageQueueGet(get_id_MsgQueue_lcd(), &msg_lcd, NULL, 100);
//		if(status==osOK){
//				dataToBuffer(msg_lcd.linea1, msg_lcd.linea2);
//		}
		
		flag = osThreadFlagsWait(, osFlagsWaitAny, osWaitForever);
		
  }
}



 void LCD_Reset(void){
	
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

 void LCD_init(void){

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

 void LCD_update(void){
	
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

static void LCD_symbolToLocalBuffer_L1(uint8_t symbol){ 
	
	/*
	En primer lugar declaramos variables auxiliares que nos ayudaran a almacenar
	valores mientras se recorre cada linea del fichero Arial12x12.h
	*/
	
  uint8_t i, value1, value2;
  uint16_t offset = 0;
	
	
	/*
	Luego debemos localizar la linea del array del fichero Arial12x12.h para poder
	recorrerla y pasar los valores a buffer, para ello declaramos una variable
	que llamaremos offset y que se "saltara" lineas dependiendo de que caracter hayamos
	seleccionado. El propio offset tien un offset que es el primer caracter ASCII, siendo
	este el ' '. Se multiplica por 25 porque cada carcter tiene 25 bytes
	*/
  
  offset = 25*(symbol - ' ');
	
	/*
	El fichero Arial12x12.h esta distribuido tal que los caracteres con numero impar 
	se corresponden con los de la primera pagina y los valores con numero par se 
	corresponden con los de la segunda página, de ahi viene los offsets de 1 y 2, para saltarse
	el primer byte de cada caracter y luego para ir de imapar en par.
	*/
  
  for(i=0;i<12;i++){
		
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
		
		/*Por ultimo escribimos los valores en el buffer el impar ira a la pagina 1 
		y el par a la linea 2*/
    
    buffer[i+positionL1] = value1; // Pagina 0
    buffer[i+128+positionL1] = value2; // Pagina 1
  }
	
	/*
	Creamos una variable que lee el primer valor de cada caracter del fichero Arial, este valor indica la anchura del caracter, asi que si vamos incrementando esa variable con el valor de la anchura
	de los diferentes caracteres. Despues se la sumamos al offset del buffer y asi el texto se va ajustando a la anchura de cada caracter.
	*/
	
  positionL1 += Arial12x12[offset];

}

static void LCD_symbolToLocalBuffer_L2(uint8_t symbol){
	
	/*
	Misma funcion que le anterior, solo que ahora se modifican los offset en buffer para que en vez de escribir en la primera linea escriba en la segunda (paginas 2 y 3)
	*/
	
  uint8_t i, value1, value2;
  uint16_t offset = 0;

  
  offset = 25*(symbol - ' ');
  
  for(i=0; i<12; i++){
    value1 = Arial12x12[offset+i*2+1];
    value2 = Arial12x12[offset+i*2+2];
    
    buffer[i+256+positionL2] = value1; // Pagina 2
    buffer[i+384+positionL2] = value2; // Pagina 3
  }
  positionL2 += Arial12x12[offset];

}

static void LCD_symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	
  if (line == 1){
    LCD_symbolToLocalBuffer_L1(symbol);
  }
	
  else if(line == 2){
    LCD_symbolToLocalBuffer_L2(symbol);
  }
	
}


 void dataToBuffer( char data_L1[32], char data_L2[32]){
	static int i, j;
  positionL1 = 0;
  positionL2 = 0;
	
	//Lee el dato de la primera linea y lo pone en el buffer
	
	for(i = 0; i < strlen(data_L1); i++){
		
		LCD_symbolToLocalBuffer(1, data_L1[i]);
	}
	
	//Ponemos a cero el buffer
	
	for(i = positionL1; i < 128; i++){
		
		buffer[i] = 0x00;
		buffer[i+128] = 0x00;
		
	}	
	
	//Lee el dato de la segunda linea y lo pone en el buffer
	
	for(j = 0; j < strlen(data_L2); j++){
		
		LCD_symbolToLocalBuffer(2, data_L2[j]);
	}
	
	//Pone a 0 la segunda linea
	
	for(j = positionL2; j < 128; j++){
		
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
		
	}
	
	//Actualizamos el LCD direcatamente aqui, asi mo tenemos que llamarla fuera de aqui.
  LCD_update();
 
}



