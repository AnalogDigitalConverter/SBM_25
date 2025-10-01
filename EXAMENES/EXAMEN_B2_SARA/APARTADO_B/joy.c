#include "cmsis_os2.h"        //Para manejar el RTOS
#include "stm32f4xx_hal.h"    //Para manejar el HAL

#include "joy.h"


static uint8_t comprobarPines(void);

MSGQUEUE_OBJ_LED pin;


uint8_t comprobarPines(){
	uint8_t pin;
	
	if     (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) pin=2; // DOWN
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) pin=4; // lEFT
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET) pin=16; // CENTER
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET)	pin=1; // UP
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) pin=8; // RIGTH
	else pin=0;
	
	return pin;
}



  void EXTI15_10_IRQHandler(void){
    
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11); ////RIGHT
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15); 
		
  }
  
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
   //osThreadFlagsSet(get_id_Th_joy, REB);
    
    pin.dir= comprobarPines();
			if (pin.dir != 0){
        osMessageQueuePut(get_id_MsgQueue_led(), &pin, 0U, 0U);
			}
      
  }
