#include "clock.h"

/*----------------------------------------------------------------------------
*      Thread 'clock': ADC 8/DIC/24: Implementación de struct de tiempo OK.
*                      ADC 15/DIC/24: Corrección de declaración de variables
*                                     para el correcto uso inter-módulos.
*---------------------------------------------------------------------------*/

#define CLOCK_TEST 0

#ifdef CLOCK_TEST
  void clock_test(void);
  void Leds_init(void);
#endif

/*---------CLOCK SOURCE TIMER--------*/
osTimerId_t id_clock_tmr;
void clock_Callback(void *args);

tiempo_t tiempo;

/*****************************************************************************/
osThreadId_t id_clock_Thread;                        // thread id
void clock_Thread (void *argument);                   // thread function

int Init_clock_Thread (void) {

  id_clock_Thread = osThreadNew(clock_Thread, NULL, NULL);
  if (id_clock_Thread == NULL) {
    return(-1);
  }

  return(0);
}

/*****************************************************************************/
void clock_Thread (void *argument) {
    tiempo.horas = tiempo.minutos = tiempo.segundos = 0; // Inicialización a cero

  id_clock_tmr = osTimerNew(clock_Callback, osTimerPeriodic, NULL, NULL);
  

#if CLOCK_TEST
  tiempo.horas     = 23;
  tiempo.minutos   = 59;
  tiempo.segundos  = 55;
#endif

    osTimerStart(id_clock_tmr, 1000U);
  while (1) {
    /**************NOTHING TO SEE HERE, OSTIMER INTERRUPT CONTROLLED**************/
    osThreadYield();
  }
}

/*****************************************************************************/
void clock_Callback(void *args)
{

  tiempo.segundos++;
  #if CLOCK_TEST
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  #endif

  /*if clk'event and clk = 1*/
  if (tiempo.segundos == 60)
  {
    tiempo.segundos = 0;
    tiempo.minutos++;
#if CLOCK_TEST
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
#endif
  }

  if (tiempo.minutos == 60)
  {
    tiempo.minutos = 0;
    tiempo.horas++;
#if CLOCK_TEST
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
#endif
  }

  if(tiempo.horas == 24)
  {
    tiempo.horas = 0;
  }
}

/*****************************************************************************/
