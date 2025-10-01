#ifndef __JOY_H
#define __JOY_H

#include "cmsis_os2.h"
#include "led.h"


#define REB 0x0001
#define IRQ 0x0002

int init_Th_joy(void);
osThreadId_t get_id_Th_joy(void);


//osMessageQueueId_t get_id_MsgQueue_lcd(void);

#endif
