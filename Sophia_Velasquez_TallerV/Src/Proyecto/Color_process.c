/*
 * Color_process.c
 *
 *  Created on: Aug 4, 2026
 *      Author: savf
 */
#include "Color_process.h"

/*Prototipo de funciones*/
ColorRGB8_t COLOR_Norm_RAW(uint16_t raw_r, uint16_t raw_g, uint16_t raw_b, uint16_t raw_c);
uint8_t COLOR_Disc(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
const char* COLOR_Clas_Bri(ColorRGB8_t color);
const char* COLOR_Clas_Neu(uint16_t raw_c);


#define FACTOR_R   1.0f
#define FACTOR_G   1.4f   //Aumenta la ganancia del verde
#define FACTOR_B   1.8f   //Aumenta la ganancia del azul

#define UMBRAL_NEGRO   800    //Todo valor de C menor a este número es NEGRO
#define UMBRAL_BLANCO  2200   //Todo valor de C mayor a este número es BLANCO

/*
 * COLOR_Norm_RAW
 * Normaliza las lecturas de 16-bits del sensor I2C a la escala normal de 0 a 255
 */
ColorRGB8_t COLOR_Norm_RAW(uint16_t raw_r, uint16_t raw_g, uint16_t raw_b, uint16_t raw_c){

    ColorRGB8_t rgb;

    if(raw_c == 0){

        rgb.r = 0; rgb.g = 0; rgb.b = 0;
        return rgb;

    }

    /*Aplicación de factores de compensación antes de mapear a 255*/
    float r_calc = ((float)raw_r / raw_c) * 255.0f * FACTOR_R;
    float g_calc = ((float)raw_g / raw_c) * 255.0f * FACTOR_G;
    float b_calc = ((float)raw_b / raw_c) * 255.0f * FACTOR_B;

    /*Limitación de valores a 255*/
    rgb.r = (r_calc > 255.0f) ? 255 : (uint8_t)r_calc;
    rgb.g = (g_calc > 255.0f) ? 255 : (uint8_t)g_calc;
    rgb.b = (b_calc > 255.0f) ? 255 : (uint8_t)b_calc;

    return rgb;

}


/*
 * COLOR_Disc
 * MODO 1: Clasifica en los 5 sectores físicos del disco del servomotor (0 a 4)
 */
uint8_t COLOR_Disc(uint16_t r, uint16_t g, uint16_t b, uint16_t c){

    if(c < UMBRAL_NEGRO)  return 4;  	//Negro (Sector 4)
    if(c > UMBRAL_BLANCO) return 3;  	//Blanco (Sector 3)

    /*Evaluación de la compensación*/
    float r_comp = r * FACTOR_R;
    float g_comp = g * FACTOR_G;
    float b_comp = b * FACTOR_B;

    if(r_comp > g_comp && r_comp > b_comp) return 0;  	//Rojo (Sector 0)
    if(g_comp > r_comp && g_comp > b_comp) return 1;  	//Verde (Sector 1)
    if(b_comp > r_comp && b_comp > g_comp) return 2;  	//Azul (Sector 2)

    return 4;
}


/*
 * COLOR_Clas_Bri
 * MODO 2: Colores Brillantes / Vivos
 * Opciones: Rojo, Verde, Azul
 */
const char* COLOR_Clas_Bri(ColorRGB8_t color){

    //Exigir un margen de diferencia (+15) evita falsos positivos de Rojo
    if(color.r > (color.g + 15) && color.r > (color.b + 15)) return "Rojo";
    if(color.g > (color.r + 15) && color.g > (color.b + 15)) return "Verde";
    if(color.b > (color.r + 15) && color.b > (color.g + 15)) return "Azul";

    return "Indefinido";

}


/*
 * COLOR_Clas_Neu
 * MODO 3: Colores Neutros
 * Opciones: Blanco, Negro, Gris, Beige, Cafe
 */
const char* COLOR_Clas_Neu(uint16_t raw_c){

    if (raw_c < UMBRAL_NEGRO)  return "Negro";
    if (raw_c > UMBRAL_BLANCO) return "Blanco";

    return "Gris";		//Si la intensidad está entre el límite del negro y del blanco, es Gris

}
