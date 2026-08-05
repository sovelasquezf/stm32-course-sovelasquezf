/*
 * stm32f4xx_it.c
 * Rutinas de servicio de interrupción
 * Autor: Sophia Alejandra Velasquez Fuentes - sovelasquezf@unal.edu.co
 */

#include "stm32f4xx_hal.h"

/*Declarar el handle de TIM11 — definido en main.c*/
extern TIM_HandleTypeDef htim11;

/*Declarar el handle de USART2 — definido en main.c*/
extern UART_HandleTypeDef huart2;


/* SysTick requerido por HAL */
void SysTick_Handler(void) {
    HAL_IncTick();
}

/* Manejador de la interrupción del Timer 11 */
void TIM1_TRG_COM_TIM11_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim11);
}

/* Manejador de la interrupción del USART2 */
void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart2);
}

/* Manejador de la interrupción del Botón del Encoder (PA4) */
void EXTI4_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


