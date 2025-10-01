#ifndef __COM_H
#define __COM_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Driver_USART.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USART_RX_FRAME_SIZE 42

void Init_USART (void);
int Init_com_Thread (void);


typedef struct{
  char trama [MAX_USART_RX_FRAME_SIZE];
  uint8_t comando;
}MSG_COM;

#endif

//uso de \0.
