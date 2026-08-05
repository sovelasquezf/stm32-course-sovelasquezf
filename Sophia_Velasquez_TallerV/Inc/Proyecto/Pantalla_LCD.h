/*
 * Pantalla_LCD.h
 *
 *  Created on: Jul 30, 2026
 *      Author: savf
 */
#ifndef PANTALLA_LCD_H_
#define PANTALLA_LCD_H_


#include <stm32f4xx_hal.h>


extern I2C_HandleTypeDef hi2c1;


#define LCD_I2C_ADDR (0x27 << 1)
#define LCD_LINE1  0x00		//Inicio línea A
#define LCD_LINE2  0x40		//Inicio línea B


void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t fila, uint8_t columna);
void LCD_Print(char *texto);
void LCD_Refresh(void);


#endif /* PANTALLA_LCD_H_ */
