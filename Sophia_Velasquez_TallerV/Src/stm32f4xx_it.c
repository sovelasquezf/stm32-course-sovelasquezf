/*
 * stm32f4xx_it.c
 * Rutinas de servicio de interrupción
 * Autor: Sophia Alejandra Velasquez Fuentes - sovelasquezf@unal.edu.co
 */

#include "stm32f4xx_hal.h"

/*Declarar el handle de TIM4 — definido en main.c*/
extern TIM_HandleTypeDef htim4;

/*Declarar el handle de ADC1 — definido en main.c*/
extern ADC_HandleTypeDef hadc1;

/*Declarar el handle de USART2 — definido en main.c*/
extern UART_HandleTypeDef huart2;

/*Manejador de SysTick — requerido por HAL para HAL_Delay() y timeouts*/
void SysTick_Handler(void){

    HAL_IncTick();

}

/*Manejador del evento de actualización de TIM4*/
void TIM4_IRQHandler(void){

    HAL_TIM_IRQHandler(&htim4);

}

/*Manejador de la interrupción del Conversor ADC1*/
void ADC_IRQHandler(void){

    HAL_ADC_IRQHandler(&hadc1);

}

/*Manejador de la interrupción del USART2*/
void USART2_IRQHandler(void){

    HAL_UART_IRQHandler(&huart2);

}









