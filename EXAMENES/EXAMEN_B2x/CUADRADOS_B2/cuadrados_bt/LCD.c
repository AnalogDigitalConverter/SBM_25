#include "LCD.h"
#include "Arial12x12.h"

unsigned char buffer[512];
 uint16_t positionL1 = 10;
 uint16_t positionL2 = 38;


GPIO_InitTypeDef GPIO_InitStruct;
TIM_HandleTypeDef htim7;

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

 

void LCD_reset(void){
	//Inicializaci?n y configuraci?n del driver SPI para gestionar el LCD
	  /* Initialize the SPI driver */
    SPIdrv->Initialize(NULL);
    /* Power up the SPI peripheral */
    SPIdrv->PowerControl(ARM_POWER_FULL);
    /* Configure the SPI to Master, 8-bit mode @20 MBits/sec */
    SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 2000000);
	
	//Configurar los pines de IO que sean necesarios y programar su valor por defecto
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_6; //RESET
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_14; //CS
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_13; //A0
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	
	//Generar la se?al de reset
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); //GPIO_PIN_SET = 1
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); //GPIO_PIN_RESET = 0
	delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	delay(1000);
	
}

void delay(uint32_t n_microsegundos){
	//Configurar y arrancar el timer para generar un evento pasados n_microsegundos
	HAL_TIM_Base_MspInit(&htim7);
	__HAL_RCC_TIM7_CLK_ENABLE();
	
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 83;
	htim7.Init.Period = n_microsegundos - 1;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.ClockDivision = 1;
	
	HAL_TIM_Base_Init(&htim7);
	HAL_TIM_Base_Start_IT(&htim7);
	
	//Esperar a que se active el flag del registro de Match correspondiente
	while(__HAL_TIM_GetCounter(&htim7) < (n_microsegundos - 1));
	
	//Borrar el flag
	__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	
	//Parar el timer y ponerlo a 0 para la siguiente llamada a la funci?n
	HAL_TIM_Base_Stop_IT(&htim7);
	HAL_TIM_Base_DeInit(&htim7);
}

void LCD_wr_data(unsigned char data){
	//CS = 0 y A0 = 1
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	
	//Escribir un dato
	SPIdrv->Send(&data,1);
	//Esperar a que se libere el bus SPI
	while(SPIdrv->GetStatus().busy);
	
	//CS = 1
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char cmd){
	// CS = 0 y A0 = 0
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	
	//Escribir un comando
	SPIdrv->Send(&cmd,1);
	//Esperar a que se libere el bus SPI
	while(SPIdrv->GetStatus().busy);
	
	//CS = 1
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

void LCD_init(void){
	LCD_wr_cmd(0xAE);//display off
	LCD_wr_cmd(0xA2);//Fija el valor de la tensi?n de polarizaci?n del LCD a 1/9
	LCD_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8);//El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22);//Fija la relaci?n de resistencias interna a 2
	LCD_wr_cmd(0x2F);//Power on
	LCD_wr_cmd(0x40);//Display empieza en la l?nea 0
	LCD_wr_cmd(0xAF);//Display ON
	LCD_wr_cmd(0x81);//Contraste
	LCD_wr_cmd(0x17);//Valor de contraste
	LCD_wr_cmd(0xA4);//Display all points normal
	LCD_wr_cmd(0xA6);//LCD Display Normal
}

void LCD_update(void){
	int i;
	LCD_wr_cmd(0x00); // 4 bits de la parte baja a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta a 0
	LCD_wr_cmd(0xB0); // Pagina 0
	
	for(i=0;i<128;i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta a 0
	LCD_wr_cmd(0xB1); // Pagina 1
	
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
	LCD_wr_cmd(0xB3); //Pagina 3
	
	for(i=384;i<512;i++){
		LCD_wr_data(buffer[i]);
	}
}

void LCD_Nada(void){
    int fila, col;
	
    for (fila = 0; fila < 4; fila++) { // Hay 4 páginas en un LCD 128x32
        for (col = 0; col < 128; col++) { // Cada página tiene 128x8 
                     buffer[fila * 128 + col] = 0x00;
							
            
        }
    }
}

void LCD_tabla(void) {
	//empieza con bloque negro
    int fila, col;
	
    for (fila = 0; fila < 4; fila++) { // Hay 4 páginas en un LCD 128x32
        for (col = 0; col < 128; col++) { // Cada página tiene 128x8 
            if (((fila % 2) == 0 && (col / 8) % 2 == 0) || ((fila % 2) == 1 && (col / 8) % 2 == 1)) {
                // Bloque negro (cuadrado lleno de píxeles)
                buffer[fila * 128 + col] = 0xFF;

            } else {
                // Bloque blanco (cuadrado vacío)
                buffer[fila * 128 + col] = 0x00;
							
            }
        }
    }
}
void LCD_tablai(void) {
	//empieza con bloque blanco
    int fila, col;
	
    for (fila = 0; fila < 4; fila++) { // Hay 4 páginas en un LCD 128x32
        for (col = 0; col < 128; col++) { // Cada página tiene 128x8 
            if (((fila % 2) == 0 && (col / 8) % 2 == 0) || ((fila % 2) == 1 && (col / 8) % 2 == 1)) {
                // Bloque negro (cuadrado lleno de píxeles)
                buffer[fila * 128 + col] = 0x00;

            } else {
                // Bloque blanco (cuadrado vacío)
                buffer[fila * 128 + col] = 0xFF;
							
            }
        }
    }
}

 void symbolToLocalBuffer_L1(uint8_t symbol){
	static uint8_t i, value1, value2;
	static uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for(i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i + positionL1] = value1;
		buffer[i + 128 + positionL1] = value2;
	}
	positionL1 = positionL1 + Arial12x12[offset];
}
 void symbolToLocalBuffer_L2(uint8_t symbol){
	static uint8_t i, value1, value2;
	static uint16_t offset = 0;
	offset = 25 * (symbol - ' ');
	for( i = 0; i < 12; i++){
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		//como queremos el segundo cuadrado en la misma pagina cambiamos estos valores
		buffer[i + 0 + positionL2] = value1;
		buffer[i + 128 + positionL2] = value2;
	}
	positionL2 = positionL2 + Arial12x12[offset];
}

 void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	if(line == 1)
		symbolToLocalBuffer_L1(symbol);
	if(line == 2)
		symbolToLocalBuffer_L2(symbol);
}


void update_texto1(char texto_L1[32]){
	static int i;
	int largo1=strlen(texto_L1);

//texto 1---------------------------------------------
	// Limpiar desde el comienzo hasta texto
	for(i = 0; i < positionL1; i++){
		buffer[i] = 0x00;
		buffer[i + 128] = 0x00;
	}	
	// Escribir cada carácter usando symbolToLocalBuffer
		for(i = 0; i < largo1; i++){
		symbolToLocalBuffer(1,texto_L1[i]);
	}

		// Limpiar desde la última posición de escritura hasta el final de la línea
			for(i = positionL1; i < 128; i++){
		buffer[i] = 0x00;
		buffer[i + 128] = 0x00;
	}	
}
void update_texto2(char texto_L2[32]){
			int largo2=strlen(texto_L2);
		static int j;
//texto2----------------------------------		
	// Limpiar desde el comienzo hasta texto	
    for(j = 0; j < positionL2; j++){
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
	}	
  // Escribir cada carácter usando symbolToLocalBuffer
		for(j = 0; j < largo2; j++){
		symbolToLocalBuffer(2,texto_L2[j]);
	}
	// Limpiar desde la última posición de escritura hasta el final de la línea
		for(j = positionL2; j < 128; j++){
		buffer[j+256] = 0x00;
		buffer[j+384] = 0x00;
	}
}
