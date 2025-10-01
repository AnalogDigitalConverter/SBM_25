#ifndef __Thread_H
#define __Thread_H

#include "cmsis_os2.h"

#define GRD 0x01
#define PQN 0x02

int init_Th_lcd(void);
osMessageQueueId_t get_id_MsgQueue_lcd(void);

typedef struct{
	char linea1 [20];
	char linea2 [20];
} MSGQUEUE_OBJ_LCD;

#endif
