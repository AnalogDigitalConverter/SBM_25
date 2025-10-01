#include "Temp.h"
#include "math.h"
#define REG_TEMP            0x00
#define TEMP_I2C_ADDR       0x48
#define BH1750_I2C_ADDR     0x23  // Dirección I2C del BH1750
#define BH1750_POWER_ON     0x01  // Comando para encender el sensor
#define BH1750_CONT_H_RES_MODE 0x10  // Modo de alta resolución continuo
//MSG QUEUE

osMessageQueueId_t temp_queue;
MSGQUEUE_TEMP_t luminosity;
void Thread_Temperatura (void *argument);


//MSGQUEUE INIT

int Init_MsgQueue_TEMP (void) {
  temp_queue = osMessageQueueNew(MSG_TEMP_OBJ, sizeof(MSGQUEUE_TEMP_t), NULL); //MAX SIZE, SIDEOF 
  if (temp_queue == NULL) {
  }
  return(0);
}



//I2C DRIVER

extern ARM_DRIVER_I2C            Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;
 
static volatile uint32_t I2C_Event;

osThreadId_t tid_Temperatura;                        // thread id
osTimerId_t tim_id_temp;



//Timer 1s Callback -> Set TEMP_FLAG

static void Timer_Temp_Callback(void const *arg){
  osThreadFlagsSet(tid_Temperatura, TEMP_FLAG);
}
 

//TIMER TEMP INIT
int Init_Tim_Temp (void) {
  osStatus_t status;
  tim_id_temp = osTimerNew((osTimerFunc_t)&Timer_Temp_Callback, osTimerPeriodic, NULL ,NULL);
  if (tim_id_temp != NULL) {
    if(status != osOK)
      return NULL;
  }
  return(0);
}

//INIT THREAD TEMP

int Init_ThTemp (void) {
	Init_MsgQueue_TEMP();
  tid_Temperatura = osThreadNew (Thread_Temperatura,NULL, NULL);
  if (tid_Temperatura == NULL) {
      return(NULL);
    }
}

// THREAD TEMP LOOP
void Thread_Temperatura (void *argument) {
    uint8_t cmd;
    uint8_t buffer_temp[2];

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

    Init_Tim_Temp();
    osTimerStart(tim_id_temp, 1000U); // Timer init

    while (1) {
        osThreadFlagsWait(TEMP_FLAG, osFlagsWaitAny, osWaitForever); // WAIT 1 S TIMER CALLBACK

        I2Cdrv->MasterReceive(BH1750_I2C_ADDR, buffer_temp, 2, false); // LEE LUMINOSIDAD
        while (I2Cdrv->GetStatus().busy) { /* Espera liberacion del bus */ }

        uint16_t raw_lux = (buffer_temp[0] << 8) | buffer_temp[1];
        float lux = raw_lux / 1.2; // Conversión a lux
				//valor comprendido entre 0 y 200 lo escalamos a 1-65535
				lux = lux *65535/200; //actually working but no me gusta el acotarlo asi
        if (lux < 1.0) {
            lux = 1.0;
        } else if (lux > 65535.0) {
            lux = 65535.0;
        }
        luminosity.temp = round(lux * 2) / 2; // Redondeo a la media unidad más cercana
        osMessageQueuePut(temp_queue, &luminosity, 0U, osWaitForever); // SET LUMINOSITY TO MSG QUEUE
    }
}

