#ifndef __LEDS_N_H
#define __LEDS_N_H

#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define LED1_ON   0x01U
#define LED2_ON   0x02U
#define LED3_ON   0x04U
#define LEDS_OFF  0x08U

int Init_leds_N (void);
void InitLeds(void);

#endif
