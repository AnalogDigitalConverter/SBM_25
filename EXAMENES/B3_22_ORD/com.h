#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#ifndef __COM_H
#define __COM_H
int Init_ThCom (void);
void myUSART_callback(uint32_t event);
typedef struct{
  char test;
} MSGQUEUE_COM_t;
#endif
