/*  P6_2 ADC 24/NOV/24    */

#include "cmsis_os2.h"                     // CMSIS RTOS header file
#include "stm32f4xx_hal.h"

#define BOUNCE_TIMEOUT_FLAG			0x01
#define EXTI_BUTTON_IRQ_FLAG		0x02
#define LONG_PRESS_FLAG				0x04
#define JOY_R_FLAG                  0x08
#define BOUNCE_TIMER_VALUE			50U
#define LONG_PRESS_TIMER_VALUE		1000U

/*----------------------------------------------------------------------------
*      Thread 'Th_joystick': Sample thread
*---------------------------------------------------------------------------*/

osThreadId_t tid_joystick;                        // thread id

/*----- Queue -----*/
osMessageQueueId_t id_MsgQueue_joy;
MSG_JOY_t joy_msg;


struct conjunto_5bits {
    unsigned int valor : 5;
};

typedef struct{
    conjunto_5bits joy_code;
}joy_msg;



static osTimerId_t Bounce_Timeout;
static osTimerId_t LONG_PRESS_Timeout;

static void Th_joystick(void *argument);
static void init_Joystick_mbed(void);
static void Bounce_Timeout_Callback (void* argument);
static void tmr_shutdown_Callback (void* argument); 

int Init_Th_joystick(void);
static uint32_t flags;
static uint8_t press = 0;

/*Thread Init from main*/
int Init_Th_joystick(void){
    /*PRODUCER*/
    tid_joystick = osThreadNew(Th_joystick/*producer*/, NULL, NULL);
    if (Th_joystick == NULL) {
        return(-1);
    }
    return(0);
}

int Init_MsgQueue_joy (void) {
    id_MsgQueue_joy = osMessageQueueNew (1, sizeof(joy_msg), NULL);
    if (mid_Queue_joy == NULL)
        return (-1);
    return(0);
}

/*Thread Init from new osThreadNew*/
void Th_joystick(void *argument){


    /*Joystick HW init*/
    init_Joystick_mbed();

    /*Bounce control Parameters*/
    Bounce_Timeout = osTimerNew(Bounce_Timeout_Callback, osTimerOnce, (void *)0, NULL);

    /*Long Press Control Parameters*/
    LONG_PRESS_Timeout = osTimerNew(LONG_PRESS_Callback, osTimerOnce, (void *)0, NULL);



    while(1){
            flags = osThreadFlagsWait((EXTI_BUTTON_IRQ_FLAG | BOUNCE_TIMEOUT_FLAG | LONG_PRESS_FLAG), osFlagsWaitAny, osWaitForever);
            switch(flags){
                case EXTI_BUTTON_IRQ_FLAG:
                    /*(de)Bounce Control (BOUNCE_TIMER_VALUE), defined as 50U (50ms); see at define section */
                    osTimerStart(Bounce_Timeout, BOUNCE_TIMER_VALUE);
                break;

                case BOUNCE_TIMEOUT_FLAG:
                    //osTimerStart(LONG_PRESS_Timeout, LONG_PRESS_TIMER_VALUE);
                    press = check_pins();
                    //joy_msg.press == press;
                    //joy_msg.timer == 0;

                    /*
                    while(press == check_pins())
                    {
                        /*Hold for LONG_PRESS_FLAG*/
                    }
                    */

                    //RELEASE UPON: Add to queue a short press
                    //osMsgQueuePUT
                    switch (press){
                        case 'U':
                            joy_msg.joy_code.valor |= (1 << 0);
                            osMessageQueuePut(id_MsgQueue_joy, &joy_msg, 0U, 0U);
                        break;
                        case 'D':
                            joy_msg.joy_code.valor |= (1 << 0);
                            joy_msg.joy_code.valor |= (1 << 1);
                            osMessageQueuePut(id_MsgQueue_joy, &joy_msg, 0U, 0U);
                        break;
                        case 'L':
                            joy_msg.joy_code.valor |= (1 << 2);
                            osMessageQueuePut(id_MsgQueue_joy, &joy_msg, 0U, 0U);
                        break;
                        case 'R':
                            joy_msg.joy_code.valor |= (1 << 1);
                            osMessageQueuePut(id_MsgQueue_joy, &joy_msg, 0U, 0U);
                        break;
                        case 'C':
                            joy_msg.joy_code.valor |= (1 << 0);
                            joy_msg.joy_code.valor |= (1 << 2);
                            osMessageQueuePut(id_MsgQueue_joy, &joy_msg, 0U, 0U);
                        break;
                    }
                case LONG_PRESS_FLAG:
                    if (press == check_pins()){
                        //long press msg assign
                        /*AÑADIR A COLA*/
                        //osMsgQueuePUT
                        switch (press){
                            case 'U':
                                break;
                            case 'D':
                                break;
                            case 'L':
                                break;
                            case 'R':
                                break;
                            case 'C':
                                break;
                    }

                break;


        }
    }
} 

/*(de)Bounce Timeout*/
static void Bounce_Timeout_Callback (void* argument) {
    osThreadFlagsSet(tid_joystick_thread, BOUNCE_TIMEOUT_FLAG);
}
/*Long Press Timeout*/
static void LONG_PRESS_Callback (void* argument) {
    osTimerStop(LONG_PRESS_Timeout);
    osThreadFlagsSet(tid_joystick_thread, LONG_PRESS_FLAG);
}

static void init_Joystick_mbed(void)
{

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15 ;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

static uint8_t check_pins(void){
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
}

void EXTI15_10_IRQHandler (void) {
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    switch(GPIO_Pin){
        case GPIO_PIN_10:
            press='U';
            break;

        case GPIO_PIN_11:
            press='R';
            break;

        case GPIO_PIN_12:
            press='D';
            break;

        case GPIO_PIN_14:
            press='L';
            break;

        case GPIO_PIN_15:
            press='C';
            break;
    }
    osThreadFlagsSet(tid_joystick, EXTI_BUTTON_IRQ_FLAG);
}


