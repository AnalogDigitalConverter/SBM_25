#include "Temp.h"

#define BH1750_I2C_ADDR     0x23  // Dirección I2C del BH1750
#define BH1750_POWER_ON     0x01  // Comando para encender el sensor
#define BH1750_CONT_H_RES_MODE 0x10  // Modo de alta resolución continuo

/*----------------------------------------------------------------------------
 *      Thlcd 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osMessageQueueId_t temp_queue;
static MSGQUEUE_TEMP_t luminosity;
void Thread_Temperatura (void *argument);

int Init_MsgQueue_TEMP (void) {
  temp_queue = osMessageQueueNew(MSG_TEMP_COUNT, sizeof(MSGQUEUE_TEMP_t), NULL);
  if (temp_queue == NULL) {
  }
  return(0);
}

//I2C DRIVER

extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

static volatile uint32_t I2C_Event;
float calc;

osThreadId_t tid_Temperatura;                        // thread id
osTimerId_t tim_id_temp;

//TIMER

static void Timer_Temp_Callback(void const *arg){
  osThreadFlagsSet(tid_Temperatura, TEMP_FLAG);
}
 
int Init_Tim_Temp (void) {
  osStatus_t status;
  tim_id_temp = osTimerNew((osTimerFunc_t)&Timer_Temp_Callback, osTimerPeriodic, NULL ,NULL);
  if (tim_id_temp != NULL) {
    if(status != osOK)
      return NULL;
  }
  return(0);
}

//Thread

osMessageQueueId_t Init_ThTemp (void) {
  Init_MsgQueue_TEMP();
  tid_Temperatura = osThreadNew (Thread_Temperatura, NULL, NULL);
  if (tid_Temperatura == NULL) {
    return(NULL);
  }
  return temp_queue;
}

void Thread_Temperatura (void *argument){
  
  uint8_t cmd;
  uint8_t buffer_temp[2];

  I2Cdrv->Initialize (NULL);
  I2Cdrv->PowerControl (ARM_POWER_FULL);
  I2Cdrv->Control      (ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  I2Cdrv->Control      (ARM_I2C_BUS_CLEAR, 0);

  // Inicialización del BH1750
  cmd = BH1750_POWER_ON;
  I2Cdrv->MasterTransmit(BH1750_I2C_ADDR, &cmd, 1, false);
  while (I2Cdrv->GetStatus().busy) { /* Espera a que el bus se libere */ }

  cmd = BH1750_CONT_H_RES_MODE;
  I2Cdrv->MasterTransmit(BH1750_I2C_ADDR, &cmd, 1, false);
  while (I2Cdrv->GetStatus().busy) { /* Espera a que el bus se libere */ }

  Init_Tim_Temp();
  osTimerStart(tim_id_temp, 1000U);  // Timer init

  while (1) {
    
    osThreadFlagsWait(TEMP_FLAG, osFlagsWaitAny, osWaitForever);  // Wait flag 1s

    I2Cdrv->MasterReceive(BH1750_I2C_ADDR, buffer_temp, 2, false);  // Lee luminosidad
    while (I2Cdrv->GetStatus().busy) { /* Espera liberacion del bus */ }

    uint16_t raw_lux = (buffer_temp[0] << 8) | buffer_temp[1];
    float lux = raw_lux / 1.2;  // Conversión a lux

    luminosity.temp = lux;
    osMessageQueuePut(temp_queue, &luminosity, 0U, osWaitForever);
  }
}
