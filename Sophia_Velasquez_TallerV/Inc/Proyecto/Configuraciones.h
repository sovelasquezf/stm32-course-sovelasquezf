/*
 * Configuraciones.h
 *
 *  Created on: Jul 31, 2026
 *      Author: savf
 */
#ifndef PROYECTO_CONFIGURACIONES_H_
#define PROYECTO_CONFIGURACIONES_H_

#include <stm32f4xx_hal.h>


extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim11;
extern UART_HandleTypeDef huart2;


extern volatile uint8_t rx_data;
extern volatile uint32_t servo_pulse;

/*Prototipos de funciones*/
void SystemClock_Config(void);
void gpio_Init(void);
void tim2_encoder_Init(void);
void tim3_servo_Init(void);
void tim11_led_ok_Init(void);
void usart2_Init(void);
void i2c1_Init(void);

#endif /* PROYECTO_CONFIGURACIONES_H_ */
