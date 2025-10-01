#ifndef __JOY_H
#define __JOY_H
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
 *      joy.h 'JOYSTICK': ADC 27/NOV/24
 *---------------------------------------------------------------------------*/
int Init_joy_Thread (void);

typedef struct {
  uint8_t button;
  int     long_press;
} MSG_JOY;


#endif
