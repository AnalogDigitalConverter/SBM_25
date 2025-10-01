#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#define RESOLUTION_12B 4096U
#define VREF 3.3f
/*----------------------------------------------------------------------------
 *      Thread 'POT_Th': ADC Potentiometer read and message generator
 *---------------------------------------------------------------------------*/
osThreadId_t tid_POT_Th;                        // thread id

void POT_Th (void *argument);                   // thread function

int Init_POT_Th (void) {
  tid_POT_Th = osThreadNew(POT_Th, NULL, NULL);
  if (tid_POT_Th == NULL) {
    return(-1);
  }
  return(0);
}

/*------------------------------THREAD------------------------------*/
void POT_Th (void *argument) {

 //hw-init
 ADC_HandleTypeDef adchandle; //handler definition
 ADC1_pins_F429ZI_config(); //specific PINS configuration
 ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration

 //Write variables
 float value_1, value_2 = 0;

  while (1) {
    value_1=ADC_getVoltage(&adchandle , 10 ); //get values from channel 10->ADC123_IN10
    value_2=ADC_getVoltage(&adchandle , 13 );

    /* GENERE UNA COLA QUE INCORPORE LOS DOS DATOS DE LOS POTES*/
    /*structmensaje.valor1 = value1*/
    pot_msg.data_adc1 = value_1;
    /*structmensaje.valor1 = value2*/
    pot_msg.data_adc2 = value_2;
    /*osMessageQueuePut()*/
    osMessageQueuePut(POT_MsgQueue, &pot_msg, 0U, osWaitForever);

    osThreadYield();                            // suspend thread
  }
}
/*--------------------------------------------------------------------------*/
/******************************************************************************
POT_1 (DIP19) = PA3 (CN9.1/A0)
POT_2 (DIP20) = PC0 (CN9.3/A1)
******************************************************************************/
void ADC1_pins_F429ZI_config()
{
  /* Config the use of analog inputs *
   * ADC123_IN10 and ADC123_IN13 and enable ADC1 clock*/
  GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_ADC1_CLK_ENABLE;

  /*PA3     ------> ADC2_IN3*/
  /***GPIO Port A CLK Enable***/
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin   = GPIO_PIN_3;  //POT_1 PA3
  GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*PC0     ------> ADC1_IN10*/
  /***GPIO Port C CLK Enable***/
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin   = GPIO_PIN_0;  //POT_2 PC0
  GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

}
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance)
{
	/*Initialize the ADC to work with single conversions.
     * 12 bits resolution, software start, 1 conversion */
   /** Configure the global features of the ADC
    * (Clock, Resolution, Data Alignment and number of conversion)*/
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
  hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    return -1;
  }
  return 0;
}

float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel)
{
  /* Configure a specific channels ang gets the voltage in float type.
   * This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick() */
  ADC_ChannelConfTypeDef sConfig = {0};
  HAL_StatusTypeDef status;
  uint32_t raw = 0;
  float voltage = 0;

/** Configure for the selected ADC regular channel
  * its corresponding rank in the sequencer and its sample time.*/
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
  {
    return -1;
  }

  HAL_ADC_Start(hadc);

  do (
      status = HAL_ADC_PollForConversion(hadc, 0); //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
  while(status != HAL_OK);

  raw = HAL_ADC_GetValue(hadc);

  /* voltage in float (resolution 12 bits and VRFE 3.3 */
  voltage = raw*VREF/RESOLUTION_12B;
  return voltage;
}

/*****************************************************************************/
/*************HEADER MATERIAL: QUEUE**************/
int Init_MsgQueue_POT (void);
osMessageQueueId_t POT_MsgQueue;

// Joysticks msgqueue ---------------------------------------------------------
typedef struct{
  uint16_t data_adc1;
  uint16_t data_adc2;
}MSGQUEUE_POT;

MSGQUEUE_POT pot_msg;

// Queue Init -----------------------------------------------------------------
int Init_MsgQueue_POT (void) {

  POT_MsgQueue = osMessageQueueNew(2, sizeof(MSGQUEUE_POT), NULL);
  if (POT_MsgQueue == NULL) {
  }
  return(0);
}

