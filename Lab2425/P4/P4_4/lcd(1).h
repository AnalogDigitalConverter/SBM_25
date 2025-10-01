#ifndef LCD_H
#define LCD_H

#define MAX_CHAR_SIZE 32
#define LCD_BUFFER_SIZE 512

#include "Driver_SPI.h"
#include "stdio.h"
#include "string.h"

void LCD_init(void);
void LCD_reset(void);
void LCD_text_update(char line_1[MAX_CHAR_LINE],
                     char line_2[MAX_CHAR_LINE]);

#endif
