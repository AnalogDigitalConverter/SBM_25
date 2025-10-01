#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "com.h"
#include "Driver_USART.h"
#include <stdio.h>

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
//Thread id
osThreadId_t tid_ThCom;                        // thread id
extern osMessageQueueId_t COM_MsgQueue;
typedef struct{
  char txt[20];
  uint32_t text_long;
}MSGQUEUE_COM;
MSGQUEUE_COM com_rec;

//Variables
extern ARM_DRIVER_USART Driver_USART3;
void ThCom (void *argument);                   // thread function


int Init_ThCom (void) {
 
  tid_ThCom = osThreadNew(ThCom, NULL, NULL);
  if (tid_ThCom == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThCom (void *argument) {
  //Iniciamos el UART como inciabamos el I2C
      static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;
    ARM_DRIVER_VERSION     version;
    ARM_USART_CAPABILITIES drv_capabilities;
    char                   cmd;
  
      /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 4800 Bits/sec */
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 19200); // bits up to 19200?
     
    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
    USARTdrv->Send("\nPress Enter to receive the time", 34);
    //osSignalWait(0x01, osWaitForever); wait de la flag a poner
  
  while (1) {
            USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
       // osSignalWait(0x01, osWaitForever);
        if (cmd == 13)                       /* CR, send greeting  */
        {
          osMessageQueueGet(COM_MsgQueue, &com_rec, NULL, 0U);
          USARTdrv->Send(com_rec.txt,com_rec.text_long);
          osDelay(1000);
          //osSignalWait(0x01, osWaitForever);
        }
  }
}

void myUSART_callback(uint32_t event){
  
  if( event & ARM_USART_EVENT_TX_COMPLETE )
       osThreadFlagsSet(tid_ThCom ,ARM_USART_EVENT_TX_COMPLETE);
    if( event & ARM_USART_EVENT_RECEIVE_COMPLETE)
      osThreadFlagsSet(tid_ThCom ,ARM_USART_EVENT_RECEIVE_COMPLETE);

}
