#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Thread.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread;                        // thread id
 
void Thread (void *argument);                   // thread function

static int init_Leds(void);
static int init_Joystick_mbed(void);

/*------OUTPUT QUEUE-----------*/
osMessageQueueId_t id_joy_MsgQueue;
MSG_JOY joy_rx_msg;

int Init_MsgQueue_joy(void) {
  id_joy_MsgQueue = osMessageQueueNew(1, sizeof(MSG_JOY), NULL);
  if (id_joy_MsgQueue == NULL) {
    return(-1);
  }
  return(0);
}
 
int Init_Thread(void){
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
  /*HW & QUEUE START CONDITION*/
	//crucial, no se pq si no peta y no asigna dirección a la cola
  if(!init_Leds() &&!init_Joystick_mbed() && !Init_MsgQueue_joy())
  {
    /*fsm in operation*/
    while(1)
    {
      osMessageQueueGet(id_joy_MsgQueue, &joy_rx_msg, NULL, osWaitForever);
      
      switch(joy_rx_msg.button)
      {
        case 'L':
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
          break;
        case 'R':
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
          break;
        case 'U':
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
          break;
        case 'D':
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_RESET);
          break;
        case 'C':
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7,GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,GPIO_PIN_SET);
          break;
      }
    }
  }
}


static int init_Leds(void)
{ 
  GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull 	= GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  return 0;
}

static int init_Joystick_mbed(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;

  //port B
  //UP DIP15 PB10 - RIGHT DIP16 PB11
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  //port E
  //DOWN DIP12 PE12 - LEFT DIP13 PE14 - CENTER DIP14 PE15
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  return(0);
}

/*****************************************************************************/



