/*
 * Servo.h
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */
#ifndef PROYECTO_SERVO_H_
#define PROYECTO_SERVO_H_

#include <stm32f4xx_hal.h>

extern TIM_HandleTypeDef htim3;


#define SERVO_MIN_PULSE    850  // Ancho de pulso para 0° (1 ms)
#define SERVO_MAX_PULSE    2350  // Ancho de pulso para 180° (2 ms)

void SERVO_Init(void);
void SERVO_SetSector(uint8_t sector);


#endif /* PROYECTO_SERVO_H_ */
