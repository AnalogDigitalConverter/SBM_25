#include "cmsis_os2.h"                          // CMSIS RTOS header file
 
/*typedef principal.h*/
extern osMessageQueueId_t RGB_Queue;
extern MSGQUEUE_RGB_t rgb_update;
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_RGB_Th;                        // thread id
 
void RGB_Th (void *argument);                   // thread function
 
int Init_RGB_Th (void) {
  tid_RGB_Th = osThreadNew(RGB_Th, NULL, NULL);
  if (tid_RGB_Th == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void RGB_Th (void *argument) {

  /*initialize*/
  init_rgb();

  uint8_t current_rgb = 0;

  while (1) {
    /*STATUS: LISTENING*/
    osMessageQueueGet(RGB_Queue, &rgb_update, NULL, 0U);

    /*STATUS: Check & Change*/
    if (current_rgb != rgb_update.color)
    {
      /*IDEA: CODIFICAR POR COLORES R,G,B,Y... MENSAJES CODIFICADOS AL USO*/
      /*Uso de ascii 'r','g','b'... y modificar los case*/

      rgb_off();
      switch(rgb_update.color)
        case 0:
          rgb_off();
        break;

        case 1:
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
        break;

        case 2:
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        break;

        case 3:
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        break;

        case 4:
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        break;
    }

    osThreadYield();                            // suspend thread
  }
}

void init_rgb(void){
  static GPIO_InitTypeDef GPIO_InitStruct;

  /* Leds Puerto D */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pin   = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void rgb_off(void)
{
  //Pull Up y nivel alto: ¿PUEDE QUE OFF SEA SET?
  //B(PD11), G(PD12), R(PD13)
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

/*Modo Reposo VERDE*/
//pwm? BARRIDO DEL 10 AL 90%
/*Varía 80% en 2 seg: cada 25ms stat++ */

/*Modo manual ROJO*/
/*Solo si entre valores: Controlar desde principal*/

/*Modo auto AZUL*/
/*Solo si entre valores: Controlar desde principal*/
