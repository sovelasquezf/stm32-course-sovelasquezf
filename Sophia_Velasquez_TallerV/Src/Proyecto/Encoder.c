/*
 * Encoder.c
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */

#include "Encoder.h"

extern TIM_HandleTypeDef htim2;

/*Variables internas para la gestión del botón (PA4)*/
static volatile uint8_t boton_flag = 0;
static volatile uint32_t last_time_bounce = 0;


/*
 * ENCODER_Init
 * Establecemos el contador inicial en un valor base (ej. 1000) para evitar números negativos al girar hacia la izquierda al arrancar
 */
void ENCODER_Init(void) {

    __HAL_TIM_SET_COUNTER(&htim2, 1000);

    boton_flag = 0;

}


/*
 * ENCODER_GetPosicion
 * Lee la posición del encoder navegando cíclicamente entre los modos (0, 1 y 2).
 * Modo seleccionado (0: Salida Disco, 1: Colores Brillantes, 2: Colores Neutros)
 */
int8_t ENCODER_GetPos(void) {

    uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim2);		//Cuenta actual

    int32_t pasos = ((int32_t)cnt - 1000) / 4;		//Son 4 flancos por click del encoder

    int8_t modo = pasos % 3;		//El paso ciclico se mantiene entre 0 y 2

    /*Manejo del módulo para números negativos si gira a la izquierda*/
    if(modo < 0){

        modo += 3;

    }

    return modo;

}


/*
 * ENCODER_BotonPress
 * Verifica si el botón (SW / PA4) fue presionado
 * 1 si fue presionado, 0 en caso contrario
 */
uint8_t ENCODER_BotonPress(void){

    if(boton_flag){

        boton_flag = 0;		//Baja la bandera

        return 1;

    }

    return 0;

}


/*
 * HAL_GPIO_EXTI_Callback
 * Manejador de la Interrupción EXTI para el botón del encoder (PA4)
 * Incluye filtro antirrebote por tiempo (200 ms)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

    if(GPIO_Pin == GPIO_PIN_4){

        uint32_t tiempo_actual = HAL_GetTick();

        /*Filtro antirrebote: Ignora pulsaciones con menos de 200 ms de diferencia*/
        if(tiempo_actual - last_time_bounce > 200){

            boton_flag = 1;

            last_time_bounce = tiempo_actual;

        }

    }

}
