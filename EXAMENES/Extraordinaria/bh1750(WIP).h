#ifndef __THTEMP_H
#define __THTEMP_H

#include "cmsis_os2.h"
#include "Driver_I2C.h"
#include "RTE_Device.h"
#include "stm32f4xx_hal.h"


#define MSG_LUX_OBJ 10 // MAX OBJ OF LUX QUEUE
#define LUX_FLAG 0x02 // FLAG TO SET AFTER TIMER CALLBACK OF 1S



typedef struct{
  float lux;
} MSGQUEUE_LUX_t; // MSGQUEUE TYPE -> FLOAT

int Init_ThTemp(void);

#endif
