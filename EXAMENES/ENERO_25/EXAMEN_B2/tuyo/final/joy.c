#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "joy.h"

/*----------------------------------------------------------------------------
 *      Thread joy.c 'JOYSTICK': ADC 28/NOV/24 : Rectificaciones tipos
 *                                                id, colas y cabeceras
 *                                                callback
 *					ADC 17/DIC/24: Alteración orden lógico para agilizar por else if
 *---------------------------------------------------------------------------*/

#define JOY_TEST 0

#define EXTI_JOY_IRQ_FLAG           0x0001 
#define BOUNCE_TIMEOUT_FLAG         0x0002

#define BOUNCE_TIMER_VALUE          50U
#define LONG_PRESS_TIMER_VALUE      950U

#if JOY_TEST
void joy_test(void);
void Leds_init(void);
MSG_JOY rx_msg;
#endif

/*------OUTPUT QUEUE-----------*/
osMessageQueueId_t id_joy_MsgQueue;
MSG_JOY joy_msg;

/*----------BUTTON TIMERS-----------*/
//Rebotes
static osTimerId_t id_rebotes_tmr;
void rebotes_tmr_Callback(void *args);

//Long-Press
static osTimerId_t id_long_press_tmr;
void long_press_tmr_Callback(void *args);

/*****************************************************************************/ 
static int init_Joystick_mbed(void);
static uint8_t check_pins(void);

osStatus_t status;


/*****************************************************************************/ 

osThreadId_t id_joy_Thread;                        // thread id
void joy_Thread (void *argument);                   // thread function

int Init_joy_Thread (void) {
  id_joy_Thread = osThreadNew(joy_Thread, NULL, NULL);
  if (id_joy_Thread == NULL) {
    return(-1);
  }
  return(0);
}

/*****************************************************************************/
//Prev value n_messages_in_queue = 2
int Init_MsgQueue_joy (void) {
  id_joy_MsgQueue = osMessageQueueNew(1, sizeof(MSG_JOY), NULL);
  if (id_joy_MsgQueue == NULL) {
    return(-1);
  }
  return(0);
}

/* thread fsm* ***************************************************************/
/*****************************************************************************/
void joy_Thread (void *argument){
  uint32_t flags = 0;

  /*HW & QUEUE START CONDITION*/
  if(init_Joystick_mbed() && !Init_MsgQueue_joy()){
		
    id_rebotes_tmr    = osTimerNew(rebotes_tmr_Callback, osTimerOnce, (void *)0, NULL);
    id_long_press_tmr = osTimerNew(long_press_tmr_Callback, osTimerOnce, (void *)0, NULL);

    /*fsm in operation*/
    while(1)
		{	
			flags = osThreadFlagsWait((EXTI_JOY_IRQ_FLAG | BOUNCE_TIMEOUT_FLAG), osFlagsWaitAny, osWaitForever);

			if (flags & BOUNCE_TIMEOUT_FLAG)
			{
				//Long-Press Control (LONG_PRESS_TIMER_VALUE), defined as 1000U (1s); see at define section
				osTimerStart(id_long_press_tmr, LONG_PRESS_TIMER_VALUE);

				//if (joystick pulsado y no larga)
				while(check_pins() && !joy_msg.long_press)
				{
					//...
				}
				osMessageQueuePut(id_joy_MsgQueue, &joy_msg, 0U, 0U);
			}
			else
			{
				osTimerStart(id_rebotes_tmr, BOUNCE_TIMER_VALUE);
			}
			
#if JOY_TEST
			joy_test();
#endif	
    }
  }
}


/*****************************************************************************/
void rebotes_tmr_Callback(void *args)
{
  osThreadFlagsSet(id_joy_Thread, BOUNCE_TIMEOUT_FLAG);
}

void long_press_tmr_Callback(void *args)
{
	if(check_pins())
	{
		joy_msg.long_press = 1;
	}
}

/*****************************************************************************/
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

  return('K');
}

/*****************************************************************************/
static uint8_t check_pins(void)
{
  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
    return('U');
  }
  else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
    return('R');
  }
  else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
    return('D');
  }
  else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
    return('L');
  }
  else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
    return('C');
  }
  else
    return(0);
}

/*****************************************************************************/
void EXTI15_10_IRQHandler (void)
{
  /*
   *     External Interrupt buttons:
   *         PB10: Joystick UP
   *         PB11: Joystick DERECHA
   *         PE12: Joystick ABAJO
   *         PB13: Boton USER
   *         PE14: Joystick IZDA
   *         PE15: Joystick CENTER
   */
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_13);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler (GPIO_PIN_15);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  //Initial value for length (clear old values)
  joy_msg.long_press = 0;

  //CODE GENERATION
  switch(GPIO_Pin)
  {
    case GPIO_PIN_10:  //UP DIP15 PB10
      joy_msg.button = 'U';//1
          break;

    case GPIO_PIN_11:  //RIGHT DIP16 PB11
      joy_msg.button = 'R'; //2
          break;

    case GPIO_PIN_12:  //DOWN DIP12 PE12
      joy_msg.button = 'D'; //3
          break;

    case GPIO_PIN_14:  //LEFT DIP13 PE14
      joy_msg.button = 'L'; //4
          break;

    case GPIO_PIN_15:  //CENTER DIP14 PE15
      joy_msg.button = 'C';//5
          break;
  }
	osThreadFlagsSet(id_joy_Thread, EXTI_JOY_IRQ_FLAG);
}

/*****************************************************************************/
/*AÑADIR FUNCIONALIDAD DE LONG PRESS AL TEST*/
/*****************************************************************************/

#if JOY_TEST
/**TEST**/
void joy_test(void)
{
#if LEDS
#else	
  Leds_init();
#endif

  status = osMessageQueueGet(id_joy_MsgQueue, &rx_msg, NULL, 0U);   // wait for message
  if (status == osOK) {
    if(check_pins())
    {
      switch(joy_msg.button)
      {
        case 'U':  //UP DIP15 PB10
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
          break;

        case 'R':  //RIGHT DIP16 PB11
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
          break;

        case 'D':  //DOWN DIP12 PE12
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
          break;

        case 'L':  //LEFT DIP13 PE14
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
          break;

        case 'C':  //CENTER DIP14 PE15
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
          break;
			}
			
		}
		else
		{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
		}
	}
}


#endif
