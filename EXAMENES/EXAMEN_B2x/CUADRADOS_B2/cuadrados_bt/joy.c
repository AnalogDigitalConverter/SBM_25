
#include "joy.h"


MSGQUEUE_OBJ_Cola pin;

  void EXTI15_10_IRQHandler(void){
    
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14); 
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15); 
		
  }
  
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

		switch (GPIO_Pin){
		
		case GPIO_PIN_10:	//arriba
      pin.joystick=1;
		
			break;
		
		case GPIO_PIN_12:	//abajo
     pin.joystick=2;

			break;
		
		case GPIO_PIN_15: //centro
	   	pin.joystick=3;
			break;
			
	}
	 osMessageQueuePut(get_id_MsgQueue_Cola(), &pin, 0U, 0U);
  }