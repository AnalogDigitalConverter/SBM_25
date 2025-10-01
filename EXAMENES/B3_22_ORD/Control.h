#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Thjoy.h"
#include "Thlcd.h"
#include "stdbool.h"

#ifndef __THCTRL_H
#define __THCTRL_H
#define FLAG_BUZZER 0x0008
#define FLAG_PWM 0x0010
#define LCD_TEMP_COUNT 10

osMessageQueueId_t Init_ThCtrl (osMessageQueueId_t JOY_Queue_id, osMessageQueueId_t TEMP_Queue_id); 
#endif
