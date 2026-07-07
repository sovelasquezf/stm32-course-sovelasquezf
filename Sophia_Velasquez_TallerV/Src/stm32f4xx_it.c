/*
 * stm32f4xx_it.c
 * Rutinas de servicio de interrupción
 * Autor: Sophia Alejandra Velasquez Fuentes - sovelasquezf@unal.edu.co
 *
 */

#include "stm32f4xx_hal.h"

/* Declarar el handle de ADC1 — definido en main.c */
extern ADC_HandleTypeDef hadc1;


/* Manejador de SysTick — requerido por HAL para HAL_Delay() y timeouts */
void SysTick_Handler(void){

    HAL_IncTick();

}

void ADC_IRQHandler(void){
	HAL_ADC_IRQHandler(&hadc1);
}
