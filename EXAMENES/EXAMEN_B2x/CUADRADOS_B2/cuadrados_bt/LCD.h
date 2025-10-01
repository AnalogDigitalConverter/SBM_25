#ifndef LCD_H
#define LCD_H

#include "stdio.h"
#include "string.h"
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h" 

void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void delay (uint32_t n_microsegundos);
void LCD_init(void);
void LCD_Nada(void);
void LCD_tabla(void);
void LCD_tablai(void);
void LCD_update(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void update_texto1(char texto_L1[32]); 
void update_texto2(char texto_L2[32]); 

#endif

