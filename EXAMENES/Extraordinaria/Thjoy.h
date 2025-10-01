#ifndef __THJOY_H
#define __THJOY_H
#include "cmsis_os2.h"
#include "RTE_Device.h"
#include "stm32f4xx_hal.h"

#define REBOTES_TIME 50U
#define FLAGS_REBOTES 0x0001U
int Init_Thread1(void);

#endif
