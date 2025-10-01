#ifndef __THREAD_H
#define __THREAD_H
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define EXTI_JOY_IRQ_FLAG           0x0001 
#define BOUNCE_TIMEOUT_FLAG         0x0002
#define SEND_FLAG										0x0004
#define BOUNCE_TIMER_VALUE          50U
#define CONTROL_PRESS_TIMER_VALUE   50U

typedef struct {
  uint8_t button;
  int     long_press;
} MSG_JOY;


int Init_Thread(void);
#endif
