#ifndef __CLOCK_H
#define __CLOCK_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file

/*----------------------------------------------------------------------------
 *      Thread 'clock': ADC 15/DIC/24: Corrección de declaración de variables
 *                                      para el correcto uso inter-módulos.
 *---------------------------------------------------------------------------*/

int Init_clock_Thread (void);

typedef struct{uint8_t horas, minutos, segundos;}tiempo_t;
extern tiempo_t tiempo;
extern osTimerId_t id_clock_tmr;

#endif
