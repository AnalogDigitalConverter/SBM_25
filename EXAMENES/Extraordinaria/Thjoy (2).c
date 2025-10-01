#include "Thjoy.h"

//estados
typedef enum {IDLE, PULSACION, LONG_PULSE} thjoy_state_t;
static thjoy_state_t state;

//Hilo Thjoy
static osThreadId_t tid_Thjoy;
extern osThreadId_t tid_ThCtrl;
static pin_puerto pulsado;
static uint32_t flags;

// timer rebotes
static void timer_rebotes_init(void);
static void timer_rebotes_start(void);
static osTimerId_t tim_id_rebotes;

//timer corta_larga
static void timer_long_pulse_init(void);
static void timer_long_pulse_start(void);
static osTimerId_t tim_id_long_pulse;
static void timer_long_pulse_stop(void);

//init
void Thjoy (void *argument);
static void Init_Joystick (void);

//mensajes y colas
osMessageQueueId_t queueJoy;
static void addItemToQueue(int pulsacion, bool corto_larga);
static void Init_MsgQueue_JOY(void);

// One-Shoot Timer Function

osMessageQueueId_t Init_Thjoy () {
  Init_MsgQueue_JOY();
  tid_Thjoy = osThreadNew(Thjoy, NULL, NULL);
	return queueJoy;
}
 
void Thjoy (void *argument) {
  //init functions
  Init_Joystick ();
  timer_rebotes_init();
  timer_long_pulse_init();

  //init variables
  state = IDLE;
  osThreadFlagsSet(tid_Thjoy, FLAGS_REBOTES);
  
  while (1) {
    switch(state) {
      case IDLE:
        osThreadFlagsWait(FLAGS_PULSACION | FLAGS_REBOTES, osFlagsWaitAll, osWaitForever); 	//se espera que haya flag de rebote y de pulsacion (activacion del pin)
        timer_long_pulse_start();																														//se inicia dos veces para que se reinicie en caso de que venga de que este parado
        timer_long_pulse_start();	
        timer_rebotes_start();
        osThreadFlagsWait(FLAGS_REBOTES, osFlagsWaitAny, osWaitForever);
        osThreadFlagsSet(tid_ThCtrl, FLAG_BUZZER);
        state = PULSACION;
      break;
      case PULSACION:
        flags = osThreadFlagsWait(FLAGS_LONG_PULSE, osFlagsWaitAny, 0);
      if (flags == FLAGS_LONG_PULSE) {  																							//FLAGS_LONG_PULSE -> PULSACION LARGA
          addItemToQueue(pulsado.pin, true);
          state = LONG_PULSE;
        } else { 																																			//(flags == FLAGS_REBOTES) o (flags == 0)
          if (HAL_GPIO_ReadPin(pulsado.puerto, pulsado.pin) == GPIO_PIN_RESET) { 			//Se suelta el boton -> PULSACION CORTA	
            addItemToQueue(pulsado.pin, false);
            timer_long_pulse_stop();
            timer_rebotes_start(); //rebotes de bajada
            state = IDLE;
          } else {  //se mantiene pulsado
            //nada (si se sigue manteniendo pulsado durante mas de 1 seg saltara la flag de long_pulse)
          }
        }
        break;
      case LONG_PULSE:
        if (HAL_GPIO_ReadPin(pulsado.puerto, pulsado.pin) == GPIO_PIN_RESET) {
          timer_rebotes_start(); //rebotes de bajada
          state = IDLE;
        }
      break;
    }
  }
}


//timer rebotes
static void Timer_rebotes_Callback (void const *arg) {
    osThreadFlagsSet(tid_Thjoy, FLAGS_REBOTES);
}
static void timer_rebotes_init(void){
  tim_id_rebotes = osTimerNew((osTimerFunc_t)&Timer_rebotes_Callback, osTimerOnce, (void*)0, NULL);
}

static void timer_rebotes_start(void) {
  osTimerStart(tim_id_rebotes, REBOTES_TIME);
}

//timer pulsacion larga
static void Timer_long_pulse_Callback (void const *arg) {
  osThreadFlagsSet(tid_Thjoy, FLAGS_LONG_PULSE);
}

static void timer_long_pulse_init(void){
  tim_id_long_pulse = osTimerNew((osTimerFunc_t)&Timer_long_pulse_Callback, osTimerOnce, (void*)0, NULL);
}

static void timer_long_pulse_start(void) {
  osTimerStart(tim_id_long_pulse, LONG_PULSE_TIME);
}

static void timer_long_pulse_stop(void) {
  osTimerStop(tim_id_long_pulse);
}

static void Init_Joystick (){
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
  
  //AZUL
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

static void Init_MsgQueue_JOY() {
  queueJoy = osMessageQueueNew(MSG_JOY_COUNT, sizeof(MSGQUEUE_JOY_t) , NULL);
}
static void addItemToQueue(int pulsacion, bool corto_larga) {
  MSGQUEUE_JOY_t msg;
  msg.pulsacion = pulsacion;
  msg.corto_larga = corto_larga;
  osMessageQueuePut(queueJoy, &msg, NULL, osWaitForever);
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
  osThreadFlagsSet(tid_Thjoy, FLAGS_PULSACION);

    switch(GPIO_Pin){
      case GPIO_PIN_10:
        pulsado.pin = GPIO_PIN_10;
        pulsado.puerto = GPIOB;
      break;
      case GPIO_PIN_11:
        pulsado.pin = GPIO_PIN_11;
        pulsado.puerto = GPIOB;
      break;
      case GPIO_PIN_12:
        pulsado.pin = GPIO_PIN_12;
        pulsado.puerto = GPIOE;
      break;
      case GPIO_PIN_14:
        pulsado.pin = GPIO_PIN_14;
        pulsado.puerto = GPIOE;
      break;
      case GPIO_PIN_15:
        pulsado.pin = GPIO_PIN_15;
        pulsado.puerto = GPIOE;
      break;
      case GPIO_PIN_13:
        pulsado.pin = GPIO_PIN_13;
        pulsado.puerto = GPIOC;
      break;
    }
  }
