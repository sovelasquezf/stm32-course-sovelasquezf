/*
 * Pantalla.c
 *
 *  Created on: Jul 28, 2026
 *      Author: savf
 */
#include "Pantalla_LCD.h"


/*Posiciones de bits según el mapeo del PCF8574*/
#define LCD_RS         0x01		//P0
#define LCD_RW         0x02		//P1 (Se mantiene en 0 para poder escribir)
#define LCD_EN         0x04		//P2 (Comienza a escribir o leer datos)
#define LCD_BACKLIGHT  0x08		//P3 (Se mantiene en 1 para encender)


/*Definiciones de RS: distinguen si se envía un comando o un dato al LCD*/
#define LCD_COMANDO  0
#define LCD_DATO     1


/*Variables usadas para la transmisión del display*/
uint8_t lcd_backlight = LCD_BACKLIGHT;			//Controla el estado de la luz de fondo del LCD
volatile uint8_t update_display_flag = 0;		//Bandera para la actualización del LCD

/*Lineas a mostrar en el display*/
char lcd_linea0[17] = "R--- G--- B---";			//Buffer para la fila 0
char lcd_linea1[17] = "Color:----------";		//Buffer para la fila 1


/*Prototipo de funciones*/
static void LCD_SendNibble(uint8_t nibble, uint8_t rs);
static void LCD_SendCommand(uint8_t comando);
static void LCD_SendData(uint8_t caracter);


/*Funciones*/
/*
 * LCD_SendNibble
 * Empaqueta 4 bits de datos (nibble) y los bits de control en un solo byte que viaja por I2C
 */
static void LCD_SendNibble(uint8_t nibble, uint8_t rs){

	/*Limpieza de los bytes control: RS, RW, E y BL*/
	uint8_t dato = (nibble & 0xF0) | lcd_backlight | rs;		//0xF0 (11110000)

	/*Envío con Enable en 0 (se queda en reposo)*/
	HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &dato, 1, 10);		//10 ms como limite de espera

	/*Cambio del Enable a 1 (Pasa a nivel alto, el HD44780 empieza a leer el dato)*/
	dato |= LCD_EN;
	HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &dato, 1, 10);

	/*Cambio del Enable a 0 (Pasa a nivel bajo, el HD44780 termina de leer el dato)*/
	dato &= ~LCD_EN;
	HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &dato, 1, 10);

}


/*
 * LCD_SendCommand
 * Traduce un byte completo en 2 llamadas de 4 bits cada una
 * Se trata sobre comandos u ordenes a seguir
 */
static void LCD_SendCommand(uint8_t comando){

	/*Aislar el nibble alto (bits 7-4)*/
	uint8_t nibble_alto = comando & 0xF0;		//0xF0 = 11110000

	/*Aislar el nibble bajo (bits 3-0) y desplazarlo a la posición alta*/
	uint8_t nibble_bajo = (comando << 4) & 0xF0;

	/*Envío de ambos nibbles como comando (RS = 0)*/
	LCD_SendNibble(nibble_alto, LCD_COMANDO);
	LCD_SendNibble(nibble_bajo, LCD_COMANDO);

}


/*
 * LCD_SendData
 * Traduce un byte completo en 2 llamadas de 4 bits cada una
 * Se trata de caracteres a dibujar
 */
static void LCD_SendData(uint8_t caracter){

	/*Aislar el nibble alto (bits 7-4)*/
	uint8_t nibble_alto = caracter & 0xF0;

	/*Aislar el nibble bajo (bits 3-0) y desplazarlo a la posición alta*/
	uint8_t nibble_bajo = (caracter << 4) & 0xF0;

	/*Envío de ambos nibbles como dato (RS = 1)*/
	LCD_SendNibble(nibble_alto, LCD_DATO);
	LCD_SendNibble(nibble_bajo, LCD_DATO);

}


/*
 * LCD_Init
 * Ejecuta la secuencia de inicialización del HD44780 en modo 4 bits,
 * Se siguen las instrucciones dadas en el datasheet (Initializing by Instruction)
 * Debe llamarse una sola vez, antes de usar cualquier otra función del driver.
 * Después de los 3 envíos de 0x30 se lleva al modo 4 bits (Esperando el primer nibble de una nueva instrucción)
 */
void LCD_Init(void){

	/*Espera inicial tras energizar (datasheet exige minimo 15 ms desde VCC estable, con margen)*/
	HAL_Delay(20);

	/*Primer envío de 0x30 (aun no se puede usar LCD_SendCommand, el LCD no sabe que esta en modo 4 bits)*/
	LCD_SendNibble(0x30, LCD_COMANDO);		//0x30 = 00110000
	HAL_Delay(5);							//Se pide esperar más de 4.1 ms

	/*Segundo envío de 0x30*/
	LCD_SendNibble(0x30, LCD_COMANDO);
	HAL_Delay(1);		//Se pide esperar más de 100 us

	/*Tercer envío de 0x30*/
	LCD_SendNibble(0x30, LCD_COMANDO);
	HAL_Delay(1);

	/*Cambio a modo 4 bits*/
	LCD_SendNibble(0x20, LCD_COMANDO);		//0x20 = 00010000 (DB4 en 0 lo pone en modo de 4 bits)
	HAL_Delay(1);

	/*Function set: 4 bits (DL = 0), 2 lineas (N = 1) y fuente 5x8 (F = 0)*/
	LCD_SendCommand(0x28);		//0x28 = 01001000

	/*Display off: Display apagado (D = 0), cursor apagado (C = 0) y blink apagado (B = 0)*/
	LCD_SendCommand(0x08);		//0x8 = 00001000

	/*Display clear: Borra toda la DDRAM y regresa el cursor a la posicion 0*/
	LCD_SendCommand(0x01);		//0x01 = 00000001
	HAL_Delay(2);				//Esta es una operacion interna lenta, requiere espera adicional

	/*Entry mode set: Incrementa el cursor automaticamente (I/D = 1) y sin desplazamiento de pantalla (S = 0)*/
	LCD_SendCommand(0x06);		//0x06 = 00000110

	/*Display on: Display encendido (D = 1), cursor apagado (C = 0) y blink apagado (B = 0)*/
	LCD_SendCommand(0x0C);		//0x0C = 00001100

}


/*
 * LCD_Clear
 * Borra el contenido del LCD y regresa el cursor a (0,0)
 */
void LCD_Clear(void){

    LCD_SendCommand(0x01);
    HAL_Delay(2);

}


/*
 * LCD_SetCursor
 * Posiciona el cursor en una fila (0-3) y columna (0-19) especifica
 * Usa las direcciones base de cada linea segun el mapa DDRAM del datasheet
 */
void LCD_SetCursor(uint8_t fila, uint8_t columna){

	uint8_t direccion;		//Dirección = address línea + # de columna

	switch(fila){

		case 0:

			direccion = LCD_LINE1 + columna;

			break;

		case 1:

			direccion = LCD_LINE2 + columna;

			break;

		default:

			direccion = LCD_LINE1 + columna;

			break;		//Va a la línea 1 en caso de llamar a una fila diferente de 0 y 1

	}

	/*Envío de la dirección con el comando Set DDRAM address*/
	LCD_SendCommand(0x80 | direccion);		//0x80 = 10000000

}


/*
 * LCD_Print
 * Escribe una cadena de texto completa a partir de la posicion actual del cursor
 * Envía cada caracter con LCD_SendData()
 */
void LCD_Print(char *texto){

	while(*texto != '\0'){

		LCD_SendData((uint8_t)(*texto));

		texto++;

	}

}


/*
 * LCD_Refresh
 * Actualiza unicamente el LCD con los 4 buffers de linea, sin transmitir por UART
 */
void LCD_Refresh(void){

	LCD_SetCursor(0, 0);
	LCD_Print(lcd_linea0);

	LCD_SetCursor(1, 0);
	LCD_Print(lcd_linea1);

}








