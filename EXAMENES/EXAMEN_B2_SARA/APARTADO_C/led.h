#ifndef __LED_H
#define __LED_H

#include "cmsis_os2.h"


int init_Th_led(void);

osThreadId_t get_id_Th_led(void);
osMessageQueueId_t get_id_MsgQueue_led(void);

typedef struct{
	uint8_t dir;
} MSGQUEUE_OBJ_LED;

#endif
