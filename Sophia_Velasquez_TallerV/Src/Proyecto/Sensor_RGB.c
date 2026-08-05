/*
 * Sensor_RGB.c
 *
 *  Created on: Jul 30, 2026
 *      Author: savf
 */
#include "Sensor_RGB.h"


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


/*Variables donde se alcamenan las lecturas del sensor*/
uint16_t tcs_c = 0;		//Intensidad de luz total (Clear)
uint16_t tcs_r = 0;  	//Componente de color Rojo
uint16_t tcs_g = 0;   	//Componente de color Verde
uint16_t tcs_b = 0;   	//Componente de color Azul


/*Prototipo de funciones privadas*/
static void TCS34725_WriteRegister(uint8_t reg, uint8_t valor);
static uint8_t TCS34725_ReadRegister(uint8_t reg);
uint8_t TCS34725_Init(void);


/*Funciones privadas*/
/*
 * TCS34725_WriteRegister
 * Escribe un byte en un registro específico del sensor mediante I2C.
 */
static void TCS34725_WriteRegister(uint8_t reg, uint8_t valor){

	uint8_t tx_data[2];

	tx_data[0] = TCS34725_CMD_BIT | reg;		//Bit 7 siempre está en 1 (0x80 = 10000000)
	tx_data[1] = valor;

	/*Transmisión de 2 bytes: [Dirección de Registro + Bit Comando, Valor a Escribir]*/
	HAL_I2C_Master_Transmit(&hi2c1, TCS34725_I2C_ADDR, tx_data, 2, 10);

}


/*
 * TCS34725_ReadReg
 * Lee un byte desde un registro del sensor, antes se le indica que registro se quiere leer
 */
static uint8_t TCS34725_ReadRegister(uint8_t reg){

	uint8_t cmd = TCS34725_CMD_BIT | reg;
	uint8_t valor = 0;

	HAL_I2C_Master_Transmit(&hi2c1, TCS34725_I2C_ADDR, &cmd, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, TCS34725_I2C_ADDR, &valor, 1, 10);

	return valor;

}


/*
 * TCS34725_Init
 * Inicializa el sensor, valida su ID y habilita la recolección de datos.
 * Retorna 1 si tuvo éxito, 0 si falló.
 */
uint8_t TCS34725_Init(void){

    uint8_t id = TCS34725_ReadRegister(TCS34725_REG_ID);

    /*Verificiación del ID*/
    if(id != 0x44 && id != 0x4D){

        return 0;		//Dispositivo no encontrado o fallo de bus I2C

    }

    /*Configuración del tiempo de integración (ATIME) y ganancia (CONTROL)*/
    TCS34725_WriteRegister(TCS34725_REG_ATIME, TCS34725_ATIME_154);			//154 ms de tiempo de integración
    TCS34725_WriteRegister(TCS34725_REG_CONTROL, TCS34725_GAIN_16X);		//Ganancia 16X

    /*Encedido del sensor*/
    TCS34725_WriteRegister(TCS34725_REG_ENABLE, TCS34725_ENABLE_PON);
    HAL_Delay(3);		//En el datasheet pide un tiempo minimo de 2.4 ms para arrancar el oscilador

    /*Activación de convertidores ADC (AEN - ADC Enable)*/
    TCS34725_WriteRegister(TCS34725_REG_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

    return 1;		//Inicialización exitosa

}


/*
 * TCS34725_Read16
 * Lee un valor completo de 16 bits desde dos registros consecutivos (Low y High)
 */
static uint16_t TCS34725_Read16(uint8_t reg_low){

    uint8_t cmd = TCS34725_CMD_BIT | reg_low;
    uint8_t buffer[2] = {0};

    /*Solicita lectura secuencial de 2 bytes a partir del registro base*/
    HAL_I2C_Master_Transmit(&hi2c1, TCS34725_I2C_ADDR, &cmd, 1, 10);
    HAL_I2C_Master_Receive(&hi2c1, TCS34725_I2C_ADDR, buffer, 2, 10);		//Como se pide 2 bytes se leen reg_low y reg_high

    /*Reconstrucción de los datos (Byte alto desplaza 8 bits a la izquierda y se une al byte bajo)*/
    return (uint16_t)((buffer[1] << 8) | buffer[0]);

}


/*
 * TCS34725_ReadRGB
 * Actualiza las variables (Buffers) con las lecturas de los 4 canales de color
 */
void TCS34725_ReadRGB(void){
    tcs_c = TCS34725_Read16(TCS34725_REG_CDATAL);
    tcs_r = TCS34725_Read16(TCS34725_REG_CDATAL + 2); // 0x16: RDATAL
    tcs_g = TCS34725_Read16(TCS34725_REG_CDATAL + 4); // 0x18: GDATAL
    tcs_b = TCS34725_Read16(TCS34725_REG_CDATAL + 6); // 0x1A: BDATAL
}
