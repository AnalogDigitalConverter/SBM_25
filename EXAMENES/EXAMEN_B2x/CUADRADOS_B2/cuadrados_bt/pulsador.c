#include "joy.h"


MSGQUEUE_OBJ_Cola pin;
int contador=0;
  void EXTI15_10_IRQHandler(void){
    
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); 
  }
  
  void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    contador++;
		switch (GPIO_Pin){
		
		case GPIO_PIN_13:	//pulsador
      pin.pulsador=contador;
		
	
	 osMessageQueuePut(get_id_MsgQueue_Cola(), &pin, 0U, 0U);
  }
}
	
