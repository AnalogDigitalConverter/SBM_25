/*P4-4  ADC 23-NOV-24*/
#include "lcd.h"
#include "Arial12x12.h"

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
static uint16_t positionL2 = 0;

void delay(uint32_t n_microsegundos);

void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void LCD_init(void);
void LCD_update(void);

void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);

void LCD_text_update(char line_1[MAX_CHAR_LINE],
                     char line_2[MAX_CHAR_LINE]);
/*------------------------------------------------------------------------------*/
/**
* @brief  Custom SW-based delay function apart from HAL
* @param  n_microsegundos: Desired delay duration in microseconds
* @retval None
*/
void delay(uint32_t n_microsegundos){

    /*TIM7(internal only) runs out of APB1*/
    __HAL_RCC_TIM7_CLK_ENABLE();
    htim7.Instance = TIM7;

    /*Current Setting 	HCLK = 168MHz
    *                       APB2Pre=HCLK_DIV2->PCLK2->84MHz
    *                       APBTimers'x2'->168MHz*/
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

void LCD_reset(void){
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

void LCD_wr_data(unsigned char data){

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

void LCD_wr_cmd(unsigned char cmd){

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

void LCD_init(void){
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

void LCD_update(void){
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

/**
* @brief  Auxiliar function for Selecting LCD buffer Line
* @param  line: LCD Buffer line selection (1 = L1, 2 = L2)
* @param  symbol: ASCII value of a given 'character',
*	bypassed to the selected buffer
* @retval None
*/
void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
    if(line == 1)
        symbolToLocalBuffer_L1(symbol);
    if(line == 2)
        symbolToLocalBuffer_L2(symbol);
}

void symbolToLocalBuffer_L1(uint8_t symbol){

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
        buffer [i + 000 + positionL1] = value1;
        buffer [i + 128 + positionL1] = value2;

    }
    positionL1 = positionL1 + Arial12x12 [offset];
}

void symbolToLocalBuffer_L2(uint8_t symbol){

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
        buffer[i + 256 + positionL2] = value1;
        buffer[i + 384 + positionL2] = value2;
    }
    positionL2 = positionL2 + Arial12x12[offset];
}


/**
* @brief  Auxiliar function for populating LCD buffer with Arial12x12 character
* @param  symbol: ASCII value of a given 'character'
* @retval None
*/
void LCD_text_update(char line_1[MAX_CHAR_SIZE], char line_2[MAX_CHAR_SIZE]){

    int i = 0;

    /*--------------------LINE 1------------------*/

    /*Line1 Text Buffering*/
    /*Buffer clear if exixst previous text (positionL1 greater than 0)*/
    for(i = 0; i < positionL1; i++){
        buffer[i] = 0x00;
        buffer[i + 128] = 0x00;
    }
    /*Buffer update loop for every character in string*/
    for(i = 0; i < strlen(line_1); i++){
        symbolToLocalBuffer(1, line_1[i]);
    }
    /*If buffer not fully populated, populate with 0 to avoid remaining old text
     *not overwritten due to inferior length*/
    for(i = positionL1; i < 128; i++){
        buffer[i] = 0x00;
        buffer[i + 128] = 0x00;
    }


    /*--------------------LINE 2------------------*/

    /*Line2 Text Buffering*/
    /*Buffer clear if exixst previous text (positionL2 greater than 0)*/
    for(i = 0; i < positionL2; i++){
        buffer[i + 256] = 0x00;
        buffer[i + 384] = 0x00;
    }
    /*Buffer update loop for every character in string*/
    for(i = 0; i < strlen(line_2); i++){
        symbolToLocalBuffer(2, line_2[i]);
    }
    /*If buffer not fully populated, populate with 0 to avoid remaining old text
     *not overwritten due to inferior length*/
    for(i = positionL2; i < 128; i++){
        buffer[i + 256] = 0x00;
        buffer[i + 384] = 0x00;
    }

    /* LCD_update integration, LCD_buffer_update will allways be called
     * upon updating the display */
    LCD_update();
}
