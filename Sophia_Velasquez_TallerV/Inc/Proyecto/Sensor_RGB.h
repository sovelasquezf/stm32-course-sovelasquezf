/*
 * Sensor_RGB.h
 *
 *  Created on: Jul 30, 2026
 *      Author: savf
 */

#ifndef PROYECTO_SENSOR_RGB_H_
#define PROYECTO_SENSOR_RGB_H_


#include <stm32f4xx_hal.h>


extern I2C_HandleTypeDef hi2c1;


/*Direccion del sensor RGB*/
#define TCS34725_I2C_ADDR   (0x29 << 1)

/*Bit COMMAND que debe ir en 1 al direccionar cualquier registro (Bit 7)*/
#define TCS34725_CMD_BIT    0x80

/*Direcciones de registros*/
#define TCS34725_REG_ENABLE  0x00
#define TCS34725_REG_ATIME   0x01
#define TCS34725_REG_CONTROL 0x0F
#define TCS34725_REG_ID      0x12
#define TCS34725_REG_STATUS  0x13
#define TCS34725_REG_CDATAL  0x14

/*Bits del registro ENABLE*/
#define TCS34725_ENABLE_PON  0x01
#define TCS34725_ENABLE_AEN  0x02

/*Bit AVALID del registro STATUS*/
#define TCS34725_STATUS_AVALID 0x01

/*Configuraciones de integración - Registro RGBC TIMING*/
#define TCS34725_ATIME_2    0xFF		//2.4 ms (1 ciclo)
#define TCS34725_ATIME_24   0xF6    	//24 ms (10 ciclos)
#define TCS34725_ATIME_101  0xD5    	//101 ms (42 ciclos)
#define TCS34725_ATIME_154  0xC0		//154 ms (64 ciclos)
#define TCS34725_ATIME_700  0x00		//700 ms (256 ciclos)

/*Configuraciones de Ganancia - Registro CONTROL*/
#define TCS34725_GAIN_1X   0x00    	//Sin ganancia
#define TCS34725_GAIN_4X   0x01    	//Ganancia 4X
#define TCS34725_GAIN_16X  0x02    	//Ganancia 16X
#define TCS34725_GAIN_60X  0x03		//Gananacia 60X

/*Buffers de Lectura (16 bits)*/
extern uint16_t tcs_c; // Intensidad de luz total (Clear)
extern uint16_t tcs_r; // Componente Rojo
extern uint16_t tcs_g; // Componente Verde
extern uint16_t tcs_b; // Componente Azul

/*Prototipo de funciones*/
uint8_t TCS34725_Init(void);
void TCS34725_ReadRGB(void);


#endif /* PROYECTO_SENSOR_RGB_H_ */
