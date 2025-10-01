#include "Thjoy.h"


int Init_MsgQueue_JOY (void);
osMessageQueueId_t JOY_MsgQueue;


// Joysticks msgqueue
typedef struct{
  int joystick;
}MSGQUEUE_JOY;
MSGQUEUE_JOY JOY_send;

int Init_MsgQueue_JOY (void) {
 
  JOY_MsgQueue = osMessageQueueNew(2, sizeof(MSGQUEUE_JOY), NULL);
  if (JOY_MsgQueue == NULL) {
  }
  return(0);
}

//Struct puertos
typedef struct pin_puerto {
  uint16_t pin;
  GPIO_TypeDef * puerto;
} pin_puerto;

osThreadId_t tid_Thjoy;

static pin_puerto pulsado;
void Init_Joystick (void);
void timer_init_joy(void);
void timer_start_joy(void);

void Thled1 (void *argument);

osTimerId_t tim_id_joy;
static uint8_t code_leds;
//timer pulsaciones

osTimerId_t tim_id_pulsaciones;
static uint32_t exec1;
int larga;
static void Timer_Pul_Callback(void const *arg){
  larga++;
}
 
int Init_Tim_Pul (void) {
  osStatus_t status;
  exec1 = 2U;
  tim_id_pulsaciones = osTimerNew((osTimerFunc_t)&Timer_Pul_Callback, osTimerPeriodic, &exec1, NULL);
  if (tim_id_pulsaciones != NULL) {
    if(status != osOK)
      return -1;
  }
  return(0);
}
//

int Init_Thread1 (void) {
  Init_Joystick ();
  tid_Thjoy = osThreadNew(Thled1, NULL, NULL);
  if (tid_Thjoy == NULL) {
      return(-1);
    }
    return(0);
}
 
void Thled1 (void *argument) {
  timer_init_joy();
  Init_MsgQueue_JOY();
  Init_Tim_Pul();
  while (1) {
    osThreadFlagsWait(FLAGS_REBOTES, osFlagsWaitAny, osWaitForever);
    timer_start_joy();
    osTimerStart(tim_id_pulsaciones,1000U);
  }
}

static void Timer1_Callback (void const *arg) {
  if (HAL_GPIO_ReadPin(pulsado.puerto, pulsado.pin) == GPIO_PIN_SET){
    timer_start_joy();
//    osTimerStart(tim_id_pulsaciones,1000U);
  } else {
    if(larga <1){
      switch (code_leds) {
        case 0:
        break;
        case 1://UP
          JOY_send.joystick = 1;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 2://RIGHT
          JOY_send.joystick = 2;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 3://LEFT
          JOY_send.joystick = 3;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 4://DOWN
          JOY_send.joystick = 4;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 5://CENTER
          JOY_send.joystick = 5;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 6://BLUE
          JOY_send.joystick = 6;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
      }
    }else{
      osTimerStop(tim_id_pulsaciones);
      larga = 0;
      switch (code_leds) {
        case 0:
        break;
        case 1://UP
          JOY_send.joystick = 7;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 2://RIGHT
          JOY_send.joystick = 8;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 3://LEFT
          JOY_send.joystick = 9;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 4://DOWN
          JOY_send.joystick = 10;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 5://CENTER
          JOY_send.joystick = 11;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
        case 6://BLUE
          JOY_send.joystick = 12;
          osMessageQueuePut(JOY_MsgQueue, &JOY_send, 0U, osWaitForever);
        break;
      }
    }
  }larga = 0;
}

void timer_init_joy(void){
  tim_id_joy = osTimerNew((osTimerFunc_t)&Timer1_Callback, osTimerOnce, (void*)0, NULL);
}

void timer_start_joy(void) {
  osTimerStart(tim_id_joy, REBOTES_TIME);
}


void Init_Joystick (){
  static GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
    //UP 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
  
    //RIGHT 
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 

    //DOWN
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct); 
  
    //LEFT
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
    //CENTER
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  // azul
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void EXTI15_10_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  osThreadFlagsSet(tid_Thjoy, FLAGS_REBOTES);
    switch(GPIO_Pin){
      case GPIO_PIN_10:
        pulsado.pin = GPIO_PIN_10;
        pulsado.puerto = GPIOB;
        code_leds = 1;
      break;
      case GPIO_PIN_11:
        pulsado.pin = GPIO_PIN_11;
        pulsado.puerto = GPIOB;
        code_leds = 2;
      break;
      case GPIO_PIN_12:
        pulsado.pin = GPIO_PIN_12;
        pulsado.puerto = GPIOE;
        code_leds = 3;
      break;
      case GPIO_PIN_14:
        pulsado.pin = GPIO_PIN_14;
        pulsado.puerto = GPIOE;
        code_leds = 4;
      break;
      case GPIO_PIN_15:
        pulsado.pin = GPIO_PIN_15;
        pulsado.puerto = GPIOE;
        code_leds = 5;
      break;
      case GPIO_PIN_13:
        pulsado.pin = GPIO_PIN_13;
        pulsado.puerto = GPIOC;
        code_leds = 6;
      break;
    }
  }
