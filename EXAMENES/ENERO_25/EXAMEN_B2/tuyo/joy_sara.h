#ifndef __JOY_H
#define __JOY_H

#include "stm32f4xx_hal.h"    //Para manejar el HAL
#include "cmsis_os2.h"

#define REB   0x01
#define IRQ   0x02
#define CHECK 0x04

typedef enum{RIGHT, LEFT, UP, DOWN, MIDDLE, NONE}dir_t;

typedef struct{
	dir_t dir;
	uint8_t dur; // 0 - pulsacion corta ; 1 - pulsacion larga
} MSGQUEUE_OBJ_JOY;

int init_Th_joy(void);
osThreadId_t get_id_Th_joy(void);
osMessageQueueId_t get_id_MsgQueue_joy(void);

#endif

