#include "stm32f4xx_hal.h" // Incluye la HAL principal
#include "stm32f4xx_it.h"   // Para la declaración de la ISR, si las ISRs están en un archivo separado
                            // Opcional: si todo va en main.c, no es estrictamente necesario,
                            // pero es buena práctica mantener ISRs en un .c diferente.

// --- 1. Declaraciones Globales y Definiciones ---

// Declarar el handle del Timer 6
TIM_HandleTypeDef htim6;

// Definiciones para el LED verde (LD1 en PB0 de la Nucleo-F429ZI)
#define LD1_GREEN_GPIO_Port GPIOB
#define LD1_GREEN_Pin       GPIO_PIN_0

// --- 2. Prototipos de Funciones ---

// Funciones de inicialización de periféricos
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);

// Manejador de errores (implementación simple)
void Error_Handler(void);

// --- 3. Implementación de Funciones ---

/**
  * @brief  Inicialización del GPIO para el LED (PB0).
  * @param  None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Habilitar el clock para GPIOB (donde está el LED1 en PB0)
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configurar el pin PB0 para el LED
    GPIO_InitStruct.Pin = LD1_GREEN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Salida Push-Pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // Sin Pull-up/Pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Baja velocidad
    HAL_GPIO_Init(LD1_GREEN_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  Inicialización del Timer 6.
  * @param  None
  * @retval None
  */
static void MX_TIM6_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim6.Instance = TIM6; // Seleccionar la instancia del Timer 6

    // Calcular Prescaler y Periodo para 250ms de interrupción
    // Frecuencia del APB1 Timer Clock para F429ZI es 90 MHz
    // F_timer = 90,000,000 / (8999 + 1) = 10,000 Hz (10 kHz)
    // Para 250ms (0.25s), necesitamos 0.25s * 10,000 Hz = 2500 pulsos.
    // Periodo = 2500 - 1 = 2499
    htim6.Init.Prescaler = 8999;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP; // Modo ascendente (cuenta de 0 a Period)
    htim6.Init.Period = 2499;
    htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // No dividir el clock interno del Timer

    // Inicializar el Timer 6 en modo base (solo contador)
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler(); // Si la inicialización falla, ir al manejador de errores
    }

    // Configuración para Master/Slave Mode (no es necesario para este uso, pero la HAL lo requiere)
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET; // No generar ningún evento de salida
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; // No operar como maestro/esclavo
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  Manejador de errores simple.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    // En caso de error, el LED parpadeará rápidamente para indicar un problema
    __disable_irq(); // Deshabilitar interrupciones para depurar
    while (1)
    {
        HAL_GPIO_TogglePin(LD1_GREEN_GPIO_Port, LD1_GREEN_Pin);
        HAL_Delay(50); // Parpadeo rápido
    }
}

// --- 4. Funciones de Callback de la HAL y Rutina de Servicio de Interrupción (ISR) ---

/**
  * @brief  Función de configuración de bajo nivel del Timer para la HAL.
  * Esta función es llamada por HAL_TIM_Base_Init() para configurar el clock y el NVIC.
  * Normalmente estaría en 'stm32f4xx_hal_msp.c'
  * @param  htim_base: Puntero al handle del Timer.
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance == TIM6)
    {
      // Habilitar el reloj para TIM6 (bus APB1)
      __HAL_RCC_TIM6_CLK_ENABLE();

      // Habilitar la interrupción global del TIM6 en el NVIC (Controlador de Interrupciones Anidadas)
      // TIM6_DAC_IRQn es la interrupción específica para el Timer 6 (compartida con el DAC)
      HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0); // Establece la prioridad (Preemptiva = 0, Subprioridad = 0)
      HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);         // Habilita la línea de interrupción del TIM6 en el NVIC
    }
}

/**
  * @brief  Rutina de Servicio de Interrupción (ISR) para el Timer 6.
  * El nombre debe coincidir EXACTAMENTE con el de la tabla de vectores del 'startup_stm32f429xx.s'.
  * Normalmente estaría en 'stm32f4xx_it.c'
  * @param  None
  * @retval None
  */
void TIM6_DAC_IRQHandler(void)
{
    // Llama a la función de manejo de interrupciones de la HAL para el Timer básico.
    // Esta función se encarga de verificar los flags de interrupción y llamar
    // a HAL_TIM_PeriodElapsedCallback() si corresponde.
    HAL_TIM_IRQHandler(&htim6);
}

/**
  * @brief  Función de callback llamada por la HAL cuando el período del Timer expira.
  * Aquí es donde se ejecuta la lógica de alternar el LED.
  * Normalmente estaría en 'stm32f4xx_it.c' o en 'main.c' si es simple.
  * @param  htim: Puntero al handle del Timer que generó la interrupción.
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Verifica si la interrupción proviene de nuestra instancia del Timer 6
    if (htim->Instance == TIM6)
    {
        HAL_GPIO_TogglePin(LD1_GREEN_GPIO_Port, LD1_GREEN_Pin); // Alternar el estado del LED
    }
}


// --- 5. Función Principal main() ---

int main(void)
{
    // Inicializa la capa HAL:
    // Configura el SysTick, la Flash, y el nivel de prioridad de interrupciones
    HAL_Init();

    // Configuración del reloj del sistema (System Clock).
    // Esta función es crucial. Sin ella, el microcontrolador no tendrá un reloj adecuado.
    // Si no usas CubeMX, deberás implementar esta función manualmente
    // o asegurarte de que tu proyecto de Keil ya tenga una configuración de reloj adecuada.
    // SystemClock_Config(); // Descomenta y asegura que esta función exista si la necesitas

    // Inicializar el GPIO para el LED
    MX_GPIO_Init();

    // Inicializar el Timer 6
    MX_TIM6_Init();

    // ¡Inicia el Timer 6 en modo interrupción!
    // Esto habilita el contador del TIM6 y activa la interrupción de actualización.
    if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
    {
        Error_Handler(); // Si el inicio falla, ir al manejador de errores
    }

    // Bucle principal de la aplicación
    while (1)
    {
        // Tu código de aplicación principal se ejecuta aquí
        // El parpadeo del LED se maneja de forma asíncrona en la interrupción.
    }
}
