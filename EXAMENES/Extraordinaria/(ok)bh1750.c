#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "math.h"
#define REG_TEMP            0x00
#define TEMP_I2C_ADDR       0x48
#define BH1750_I2C_ADDR     0x23  // Dirección I2C del BH1750
#define BH1750_POWER_ON     0x01  // Comando para encender el sensor
/*CONSIDERAR TIMER Y MEDIDAS CADA CALLBACK: NO MEDIDA CONTINUA*/
#define BH1750_CONT_H_RES_MODE 0x10  // Modo de alta resolución continuo
#define MEAS_FLAG           0x02
#define LUX_TYP_COEF        1.2f

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_BH1750_Th;                        // thread id
 
void BH1750_Th (void *argument);                   // thread function
 
int Init_Thread (void) {
  tid_BH1750_Th = osThreadNew(BH1750_Th, NULL, NULL);
  if (tid_BH1750_Th == NULL) {
    return(-1);
  }
  return(0);
}

/**----------QUEUE DEPENDENCIES------------*/
osMessageQueueId_t lux_queue;

MSGQUEUE_LUX_t luminosity;

/*----------MSGQUEUE INIT---------------*/
int Init_LUX_MsgQueue (void) {
  lux_queue = osMessageQueueNew(2, sizeof(MSGQUEUE_LUX_t), NULL);
  if (lux_queue == NULL) {
  }
  return(0);
}

/*******************************I2C********************************/
extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
static volatile uint32_t I2C_Event;


/*-----local variables-----*/
static uint8_t buffer_temp[2];
static uint16_t raw_lux;
static float lux;


/*---------------------------------------------------------------------------*/
 
void BH1750_Th (void *argument) {
  /*******************PENDING TREATMENT**************/
  i2c_lux_init();

  Meas_Timer_init();
  osTimerStart(Meas_Timer, 1000U); // Timer init
 
  while (1) {
  /*STATUS: IDLE*/
    //Measure Timer Callback
    osThreadFlagsWait(MEAS_FLAG, osFlagsWaitAny, osWaitForever); // WAIT 1 S TIMER CALLBACK

  /*STATUS: READ*/
    // Puebla buffer de lectura
    I2Cdrv->MasterReceive(BH1750_I2C_ADDR, buffer_temp, 2, false);
    while (I2Cdrv->GetStatus().busy) { /* Espera liberacion del bus */ }
    //Creacion reiterada de variable: generar variable estatica fuera de bucle
    raw_lux = (buffer_temp[0] << 8) | buffer_temp[1];

  /*STATUS: CALC & TX*/
    if (lux_math_calc())
    {
      osMessageQueuePut(lux_queue, &luminosity, 0U, osWaitForever); // SET LUMINOSITY TO MSG QUEUE
    }

    osThreadYield();                            // suspend thread
  }
}



void i2c_lux_init(void *argument)
{
  uint8_t cmd;

  /*CREAR FUNCION DE INICIALIZACIÓN HW Y COMANDOS DE I2C*/
  /******************************************************/
  I2Cdrv->Initialize(NULL);
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);

  // Inicialización del BH1750
  cmd = BH1750_POWER_ON;
  I2Cdrv->MasterTransmit(BH1750_I2C_ADDR, &cmd, 1, false);
  while (I2Cdrv->GetStatus().busy) { /* Espera a que el bus se libere */ }

  cmd = BH1750_CONT_H_RES_MODE;
  I2Cdrv->MasterTransmit(BH1750_I2C_ADDR, &cmd, 1, false);
  while (I2Cdrv->GetStatus().busy) { /* Espera a que el bus se libere */ }
  /******************************************************/
}

int lux_math_calc(void)
{
  //default
  luminosity.lux = 0;

  /**********MATHS**********/
  //Tratamientos numericos: Integrar a funcion
  lux = raw_lux / LUX_TYP_COEF; // Conversión a lux
  //valor comprendido entre 0 y 200 lo escalamos a 1-65535
  lux = lux * 65535/200; //currently working but no me gusta el acotarlo asi
  if (lux < 1.0) {
    lux = 1.0;
  } else if (lux > 65535.0) {
    lux = 65535.0;
  }
  luminosity.lux = round(lux * 2) / 2; // Redondeo a la media unidad más cercana
  /**********MATHS**********/
  return luminosity.lux;
}

//Timer 1s Callback -> Set MEAS_FLAG
static void Meas_Timer_Callback(void const *arg){
  osThreadFlagsSet(tid_BH1750_Th, MEAS_FLAG);
}

//TIMER Meas INIT
int Meas_Timer_init (void) {
  osStatus_t status;
  tim_id_meas = osTimerNew((osTimerFunc_t)&Meas_Timer_Callback, osTimerPeriodic, NULL ,NULL);
  if (tim_id_meas != NULL) {
    if(status != osOK)
      return NULL;
  }
  return(0);
}
