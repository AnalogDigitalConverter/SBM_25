#ifndef __COLA_H
#define __COLA_H

#include "cmsis_os2.h"        //Para manejar el RTOS
#include "stm32f4xx_hal.h"    //Para manejar el HAL
#include "lcd.h"

int init_Th_Cola(void);

osThreadId_t get_id_Th_Cola(void);
osMessageQueueId_t get_id_MsgQueue_Cola(void);

typedef struct{
	uint8_t joystick;
} MSGQUEUE_OBJ_Cola;

#endif
