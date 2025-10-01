#ifndef __Thread_H
#define __Thread_H

#include "cmsis_os2.h"
#include <stdio.h>
#include <string.h>
#include "Driver_SPI.h"
#include "stm32f4xx_hal.h"


#define MSGQUEUE_OBJECTS_LCD 4

typedef struct{
	uint8_t dir;
} MSGQUEUE_OBJ_LCD;

int init_Th_lcd(void);
osMessageQueueId_t get_id_MsgQueue_lcd(void);

#endif
