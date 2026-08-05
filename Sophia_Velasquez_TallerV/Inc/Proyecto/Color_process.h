/*
 * Color_process.h
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */

#ifndef PROYECTO_COLOR_PROCESS_H_
#define PROYECTO_COLOR_PROCESS_H_

#include <stdint.h>

/*Estructura para almacenar los datos transformados a escala 0 - 255*/
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ColorRGB8_t;

/*Prototipos de funciones*/
ColorRGB8_t COLOR_Norm_RAW(uint16_t raw_r, uint16_t raw_g, uint16_t raw_b, uint16_t raw_c);
uint8_t COLOR_Disc(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
const char* COLOR_Clas_Bri(ColorRGB8_t color);
const char* COLOR_Clas_Neu(uint16_t raw_c);


#endif /* PROYECTO_COLOR_PROCESS_H_ */
