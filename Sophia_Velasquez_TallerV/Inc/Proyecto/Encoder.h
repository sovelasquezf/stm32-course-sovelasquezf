/*
 * Encoder.h
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */

#ifndef PROYECTO_ENCODER_H_
#define PROYECTO_ENCODER_H_

#include <stm32f4xx_hal.h>


/*Numero de modos: 3*/
#define ENCODER_VALOR_MIN       0
#define ENCODER_VALOR_MAX       2


typedef enum{
    MODO_DISC = 0,
    MODO_COLOR_BRILL,
    MODO_COLOR_NEU
} Modo;


void ENCODER_Init(void);
int8_t ENCODER_GetPos(void);
void ENCODER_SetPos(int8_t pos);
uint8_t ENCODER_BotonPress(void);
void ENCODER_IRQHandler(uint16_t GPIO_Pin);

#endif /* PROYECTO_ENCODER_H_ */
