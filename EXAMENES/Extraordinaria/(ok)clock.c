#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "clock.h"

/*----------------------------------------------------------------------------
 *      Thread 'CLK_Th': Clock FSM with manual and serial frame programming
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_CLK_Th;                        // thread id
void CLK_Th (void *argument);                   // thread function

/*----------clock timer-------------*/
osTimerId_t tim_tick;

/*----------variables globales-------------*/
extern uint8_t seg, min, hour;


/*---------------------------------------------------------------------------*/
int Init_CLK_Th (void) {
 
	tid_CLK_Th = osThreadNew(CLK_Th, NULL, NULL);
  if (tid_CLK_Th == NULL) {
    return(-1);
  }
 
  return(0);
}

void CLK_Th (void *argument) {
  
  init_clk_tick();
  
  while (1)
  {
    /************************/
    /*  nothing to see here */
    /************************/
  }

}

/*---------------------------------------------------------------------------*/

static void Tick_Callback (void const *arg) {
  if (seg == 59)
  {
    seg = 0;
    min++;
  }
  else if (seg < 59)
  {
    seg++;
  }
  if (min == 60)
  {
    min = 0;
    hour++;
  }
  if(hour == 24)
  {
    hour = 0;
  }
}
 
/*---------------------------------------------------------------------------*/
void init_clk_tick(void *argument)
{
  tim_tick = osTimerNew((osTimerFunc_t)&Tick_Callback, osTimerPeriodic, (void*)0, NULL);
  osTimerStart(tim_tick, 1000U);
}
