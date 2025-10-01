#ifndef __LCD_H
#define __LCD_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "Driver_SPI.h"


void LCD_reset(void);
void delay(uint32_t n_microsegundos);
void LCD_Init(void);
void LCD_update(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd);
void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void LCD_update_L1(void);
void LCD_update_L2(void);
void escribir_cadena(char cadena[], int linea);
void borrar_todo_lcd(void);
void borrar_linea_lcd(uint16_t linea);

void mySPI_callback(uint32_t event);

extern unsigned char buffer[512];

int Init_lcd (void);

typedef struct{
  uint16_t linea;
  char cadena[50];
}MSG_LCD;

#endif