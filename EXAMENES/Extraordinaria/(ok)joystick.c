/*-----------------------CODIGO SUSCEPTIBLE DE header------------------------*/
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Thjoy.h"

#define LONG_PRESS_PARAM 1000U
#define FLAG_PULSACION   0x01
#define FLAG_REBOTES     0x02
#define FLAG_LONG_PRESS  0x04

//Struct ports
typedef struct pin_port {
  GPIO_TypeDef * gpio_port;
  uint16_t       pin;
  bool           length_long;
} pin_port;
static pin_port pressing;


int Init_MsgQueue_JOY (void);
osMessageQueueId_t JOY_MsgQueue;

// Joysticks msgqueue
typedef struct{
  uint8_t button;
  bool long_press;
}MSGQUEUE_JOY;
MSGQUEUE_JOY Joy_msg;

/*------BUTTON TIMERS-------*/
//Rebotes
osTimerId_t tim_id_reb_joy;
//Long-Press
osTimerId_t tim_id_press_joy;

/*-----------------------CODIGO SUSCEPTIBLE DE header------------------------*/

int Init_MsgQueue_JOY (void) {
  JOY_MsgQueue = osMessageQueueNew(2, sizeof(MSGQUEUE_JOY), NULL);
  if (JOY_MsgQueue == NULL) {
  }
  return(0);
}

/*****************************************************************************/

/*----------------------------------------------------------------------------
 *      Thread  'JOY_Th': Joystick FSM & packet generator
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_JOY_Th;                        // thread id
 
void JOY_Th (void *argument);                   // thread function

/* main callee *******************************************/
/* set Init_Thread to desired function name to be called */
/*********************************************************/
int Init_Thread (void) {
 
  tid_JOY_Th = osThreadNew(Thread, NULL, NULL);
  if (tid_JOY_Th == NULL) {
    return(-1);
  }
 
  return(0);
}

/* thread fsm* *******************************************/
/*********************************************************/
void JOY_Th (void *argument) {
  /*initial conditions*/
  Init_Joystick();
  reb_timer_init_joy();
  press_timer_init_joy();
  
  /*fsm in operation*/
  while (1) {
    /*---------STATE: IDLE---------------*/
	osThreadFlagsWait(FLAG_PULSACION, osFlagsWaitAny, osWaitForever);
	osThreadFlagsWait(FLAG_REBOTES, osFlagsWaitAny, osWaitForever);

    /*---------STATE: PRESSED---------*/
	while((HAL_GPIO_ReadPin(pressing.gpio_port, pressing.pin) == GPIO_PIN_SET) && (!pressing.length_long))
	{
    //TimedOut flag check, avoiding blocked thread allowing button release check
	  osThreadFlagsWait(FLAG_LONG_PRESS, osFlagsWaitAny, 50U);
	}

	/*Output, variables already dealt with at callbaks*/
	osMessageQueuePut(JOY_MsgQueue, &Joy_msg, 0U, osWaitForever);

/*----------------------------MAYBE NOT REQUIRED--------------------------------*/
/*	//Button released upon long threshold
    if(!pressing.length_long)
	{
	  //Send short message
      pressing.length_long = false;
      osMessageQueuePut(JOY_MsgQueue, &Joy_msg, 0U, osWaitForever);
	}
*/
/*----------------------------MAYBE NOT REQUIRED--------------------------------*/


    osThreadYield();                            // suspend thread
  }
}

/*------TIMER CALLBACKS-------*/
static void Rebound_Callback (void const *arg) {
  //Check data integrity and attach value to packet
  if (code_leds == 'u' && code_leds == 'd' && code_leds == 'l' && code_leds == 'r' && code_leds == 'c')
  {
    //code confirmation and message attach
    Joy_msg.button = code_leds;
  }

  /*Clear possible multiple callouts to IRQ due to rebound*/
  osThreadFlagsClear(tid_Thjoy, FLAG_PULSACION);
}

static void Long_Press_Callback(void const *arg)
{
  if (HAL_GPIO_ReadPin(pressing.gpio_port, pressing.pin) == GPIO_PIN_SET)
  {
    //Change press status to long after 1sec callback
    pressing.length_long = true;

    //Generate message in order to not wait to release?
    //osMessageQueuePut(JOY_MsgQueue, &Joy_msg, 0U, osWaitForever);
  }
}

/*************************************/

/* rebound timer dependencies */
void reb_timer_init_joy(void){
  tim_id_reb_joy = osTimerNew((osTimerFunc_t)&Rebound_Callback, osTimerOnce, &FLAG_REBOTES, NULL);
}

void reb_timer_start_joy(void) {
  osTimerStart(tim_id_reb_joy, REBOTES_TIME);
}
/*************************************/

/* long press timer dependencies */
void press_timer_init_joy(void){
  tim_id_press_joy = osTimerNew((osTimerFunc_t)&Long_Press_Callback, osTimerOnce, &FLAG_LONG_PRESS, NULL);
}

void press_timer_start_joy(void) {
  osTimerStart(tim_id_press_joy, LONG_PRESS_PARAM);
}
/*************************************/


/** hw initialiation**********************************************************/
void Init_Joystick ()
{
  static GPIO_InitTypeDef GPIO_InitStruct;
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*****EXPERIMENTAL: COMMON CONFIG PRESET*****/
  GPIO_InitStruct{
    .Mode = GPIO_MODE_IT_RISING
    .Pull = GPIO_PULLDOWN;
  };
  
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
  
}

void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  //FLAG for fsm control
  osThreadFlagsSet(tid_Thjoy, FLAG_PULSACION);

  //Rebound timer
  reb_timer_start_joy();

  //Initial value for length (clear old values)
  pressing.length_long = false;

  //CODE GENERATION
  switch(GPIO_Pin)
  {
    case GPIO_PIN_10:  //UP DIP15 PB10
      pressing.pin = GPIO_PIN_10;
      pressing.gpio_port = GPIOB;
      code_leds = 'u';
    break;

    case GPIO_PIN_11:  //RIGHT DIP16 PB11
      pressing.pin = GPIO_PIN_11;
      pressing.gpio_port = GPIOB;
      code_leds = 'r';
    break;

    case GPIO_PIN_12:  //DOWN DIP12 PE12
      pressing.pin = GPIO_PIN_12;
      pressing.gpio_port = GPIOE;
      code_leds = 'd';
    break;

    case GPIO_PIN_14:  //LEFT DIP13 PE14
      pressing.pin = GPIO_PIN_14;
      pressing.gpio_port = GPIOE;
      code_leds = 'l';
    break;

    case GPIO_PIN_15:  //CENTER DIP14 PE15
      pressing.pin = GPIO_PIN_15;
      pressing.gpio_port = GPIOE;
      code_leds = 'c';
    break;
  }
}
