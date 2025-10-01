#ifndef __THTEMP_H
#define __THTEMP_H

#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"
#include "stm32f4xx_hal.h"


#define MSG_TEMP_OBJ 10 // MAX OBJ OF TEMP QUEUE
#define TEMP_FLAG 0x02 // FLAG TO SET AFTER TIMER CALLBACK OF 1S



typedef struct{
  float temp;
} MSGQUEUE_TEMP_t; // MSGQUEUE TYPE -> FLOAT

int Init_ThTemp(void);

#endif
