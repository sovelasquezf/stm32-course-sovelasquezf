/*
 * Servo.c
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */
#include "Servo.h"
#include <stdint.h>

#define SERVO_TIMER_CHANNEL  TIM_CHANNEL_1

#define SERVO_MIN_PULSE    1000		//Ancho de pulso para 0° (1 ms)
#define SERVO_MAX_PULSE    2000  	//Ancho de pulso para 180° (2 ms)

#define SERVO_NUM_SECTORES  5


/* Prototipo de funciones*/
static void SERVO_SetPulso(uint16_t pulso_us);


/*
 * SERVO_SetPulso
 * Escribe directamente el ancho de pulso (en microsegundos) en el CCR1 de TIM3.
 * Asume que el timer está configurado a 1 MHz (1 tick = 1 µs) con período de 20 ms (ARR = 19999).
 */
static void SERVO_SetPulso(uint16_t pulso_us){

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulso_us);

}


/*
 * SERVO_Init
 * Arranca la generación del canal PWM en TIM3 (Pin PA6).
 */
void SERVO_Init(void){

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

}


/*
 * SERVO_SetSector
 * Mueve la aguja del servo al centro del sector indicado (0 a 4).
 * Divide el rango del pulso en 5 partes iguales y posiciona el servo en el punto medio de cada una.
 */
void SERVO_SetSector(uint8_t sector){

    /*Limitación del sector para evitar salir del rango de 0 a 4 */
    if(sector >= SERVO_NUM_SECTORES){

        sector = SERVO_NUM_SECTORES - 1; 	//Va al último sector

    }

    uint16_t rango = SERVO_MAX_PULSE - SERVO_MIN_PULSE;		//2000 us - 1000 us = 1000 us

	uint16_t ancho_sector = rango / SERVO_NUM_SECTORES;		//1000 us / 5 = 200 us

	uint16_t inicio_sector = sector * ancho_sector;

	uint16_t centro_relativo = inicio_sector + (ancho_sector / 2);		//Para apuntar al centro se desplaza la mitad del ancho

	uint16_t pulso = SERVO_MIN_PULSE + centro_relativo;		//Se suma esta ubicación al pulso minimo

    SERVO_SetPulso(pulso);

}
