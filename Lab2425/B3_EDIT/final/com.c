#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "com.h"

/*----------------------------------------------------------------------------
 *      Thread com: USART Communications Thread
 *								13/DIC/24
 *---------------------------------------------------------------------------*/

osThreadId_t id_com_receive;                        // thread id
osThreadId_t id_com_send;                        		// thread id

osMessageQueueId_t msg_comReceive;
osMessageQueueId_t msg_comSend;

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

void Com_Receive (void *argument);                   // thread function
void Com_Send (void *argument);                   // thread function
void limpiar_usart_msg(MSG_COM *msg);

void myUSART_callback(uint32_t event);

int Init_com_Thread (void) {

  msg_comReceive = osMessageQueueNew(16, sizeof(MSG_COM),NULL);
  msg_comSend = osMessageQueueNew(16, sizeof(MSG_COM),NULL);

  id_com_receive = osThreadNew(Com_Receive, NULL, NULL);
  if (id_com_receive == NULL) {
    return(-1);
  }

  id_com_send = osThreadNew(Com_Send, NULL, NULL);
  if (id_com_send == NULL) {
    return(-1);
  }

  return(0);
}

void Com_Receive (void *argument) {

  Init_USART ();
  
  static char cmd;
  
  static uint8_t cont, frame_size = 0;
  bool comienzo = false;
  
  MSG_COM msg_Re;
  limpiar_usart_msg(&msg_Re);
  
  while (1) {
    //control de validez del paquete y concordancia tamaño con campo payload
    USARTdrv->Receive(&cmd, 1); /* Get byte from UART */
    osThreadFlagsWait(0x20, osFlagsWaitAny,osWaitForever);

    if(!comienzo){
      if(cmd == 0x01){
        comienzo = true;
      }
    }

    if (comienzo){
      msg_Re.trama[cont] = cmd;
      cont++;
			if (cont == 3){frame_size = cmd;}
      if (cmd == 0xFE && cont == frame_size){
        comienzo = false;
        msg_Re.comando = msg_Re.trama[1];
        osMessageQueuePut(msg_comReceive, &msg_Re, 0, osWaitForever);
        limpiar_usart_msg(&msg_Re);
        cont = 0;
      }
    }
  }
}

void Com_Send (void *argument) {
  
  MSG_COM msg_Se;
  int8_t send_index = -1;
  
  while (1) {
    limpiar_usart_msg(&msg_Se);
    osMessageQueueGet(msg_comSend, &msg_Se, 0, osWaitForever);
    msg_Se.trama[0] = 0x01;

    USARTdrv->Send(&msg_Se.trama, sizeof(msg_Se.trama)); /* Get byte from UART */
    osThreadFlagsWait(0x40, osFlagsWaitAny,osWaitForever);

  }
}

void Init_USART (void){
  
  /*Initialize the USART driver */
  USARTdrv->Initialize(myUSART_callback);
  
  /*Power up the USART peripheral */
  USARTdrv->PowerControl(ARM_POWER_FULL);

  /*Configure the USART to 115200 Bits/sec */
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
  ARM_USART_DATA_BITS_8 |
  ARM_USART_PARITY_NONE |
  ARM_USART_STOP_BITS_1 |
  ARM_USART_FLOW_CONTROL_NONE, 115200);
  
  /* Enable Receiver and Transmitter lines */
  USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
  USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
  
}

void myUSART_callback(uint32_t event){
  //TX salta por cada byte, si mando multi-byte me jode la transferencia. Usar solo SEND_COMPLETE
  //uso de string.h para usar \0 y strlen
  uint32_t mask_rx;
  uint32_t mask_tx;
  mask_rx = ARM_USART_EVENT_RECEIVE_COMPLETE;
  mask_tx = ARM_USART_EVENT_TRANSFER_COMPLETE | ARM_USART_EVENT_SEND_COMPLETE;
  
  if (event & mask_rx) {
    /* Success: Wakeup Thread */
    osThreadFlagsSet(id_com_receive, 0x20);
  }

  if (event & mask_tx) {
    /* Success: Wakeup Thread */
    osThreadFlagsSet(id_com_send, 0x40);
  }

  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    //__breakpoint(0); 
    /* Error: Call debugger or replace
     *  with custom error handling */
  }
  if (event & (ARM_USART_EVENT_RX_OVERFLOW |
    ARM_USART_EVENT_TX_UNDERFLOW)) {
    //__breakpoint(0); 
    /* Error: Call debugger or replace
     *  with custom error handling */
    }
}


//sustituir por strings que son punteros (* (char) 
void limpiar_usart_msg(MSG_COM *msg){
  uint8_t pos;
  msg->comando = 0x00;
  for (int i = 0; i < MAX_USART_RX_FRAME_SIZE; i++){msg->trama[i] = 0x00;}
}
