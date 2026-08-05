/*
 * Main.c
 *
 *  Created on: Jul 30, 2026
 *      Author: savf
 */
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* Inclusiones exactas según imagen.png */
#include "Configuraciones.h"
#include "Encoder.h"
#include "Pantalla_LCD.h"
#include "Sensor_RGB.h"
#include "Servo.h"
#include "Color_process.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim11;

typedef enum {
    ESTADO_MENU,
    ESTADO_ESPERA,
    ESTADO_MEDICION
} EstadoFSM_t;

EstadoFSM_t estado_actual = ESTADO_MENU;

/*Banderas*/
volatile uint8_t usart_done = 0;
volatile uint8_t boton_presionado = 0;

/*Variables de control*/
extern volatile uint8_t rx_data;
uint8_t modo_seleccionado = 0; // 0: Disco, 1: Brillantes, 2: Neutros
uint8_t modo_previo = 255;     // Para evitar parpadeos en la LCD
uint8_t refrescar_pantalla = 1;

char uart_buf[256];


int main(void) {
    /*Inicialización*/
    HAL_Init();
    SystemClock_Config();
    gpio_Init();
    tim2_encoder_Init();
    tim3_servo_Init();
    tim11_led_ok_Init();
    usart2_Init();
    i2c1_Init();

    LCD_Init();
    TCS34725_Init();
    ENCODER_Init();
    SERVO_Init();

    for (uint8_t i = 0; i < 5; i++) {
            SERVO_SetSector(i);
            HAL_Delay(500); // Espera 500 ms en cada sector
        }

        // Regresa al centro (Sector 2 - 90°)
        SERVO_SetSector(2);
        HAL_Delay(500);

    /*Comienzo de recepción y transmisión de mensaje inicial*/
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_data, 1);

    snprintf(uart_buf, sizeof(uart_buf),
     "---------------------------------------\r\n"
	 " Proyecto - Sophia Velasquez\r\n"
	 "---------------------------------------\r\n"
	 " Sistema Iniciado\r\n"
     " Navegue por el menu con el Encoder\r\n"
	 "---------------------------------------\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 200);

    while (1) {

    	switch(estado_actual){

		case ESTADO_MENU:		//Obtiene el modo directamente desde Encoder.c (retorna 0, 1 o 2)

			modo_seleccionado = (uint8_t)ENCODER_GetPos();

			if(modo_seleccionado != modo_previo || refrescar_pantalla){

				modo_previo = modo_seleccionado;
				refrescar_pantalla = 0;

				LCD_Clear();
				LCD_SetCursor(0, 0);
				LCD_Print("ELIJA EL MODO:");

				LCD_SetCursor(1, 0);

				if(modo_seleccionado == 0)      LCD_Print("> 1:DISCO       ");
				else if(modo_seleccionado == 1) LCD_Print("> 2:BRILLANTES  ");
				else                            LCD_Print("> 3:NEUTROS     ");

			}

			if(ENCODER_BotonPress()){		//Verifica la pulsación del botón usando la función con antirrebote de Encoder.c

				refrescar_pantalla = 1;
				estado_actual = ESTADO_ESPERA;

			}

			break;

		case ESTADO_ESPERA:

			if(refrescar_pantalla){

				refrescar_pantalla = 0;

				LCD_Clear();
				LCD_SetCursor(0, 0);

				if(modo_seleccionado == 0)      LCD_Print("MODO:DISCO      ");
				else if(modo_seleccionado == 1) LCD_Print("MODO:BRILLANTES ");
				else                            LCD_Print("MODO:NEUTROS    ");

				LCD_SetCursor(1, 0);
				LCD_Print("Envie M p/leer  ");

			}

			if(ENCODER_BotonPress()){		//Volver al menú si presiona nuevamente el encoder

				refrescar_pantalla = 1;
				estado_actual = ESTADO_MENU;

			}

			if(usart_done){		//Evaluación de la recepción UART

				usart_done = 0;		//Bajar bandera

				if(rx_data == 'M'){

					estado_actual = ESTADO_MEDICION;

				}

				HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_data, 1);		//Reactivar interrupción UART para la siguiente tecla

			}

			break;

		case ESTADO_MEDICION:

			TCS34725_ReadRGB();		//Leer sensor I2C (Variables globales tcs_r, tcs_g, tcs_b, tcs_c)

			ColorRGB8_t rgb255 = COLOR_Norm_RAW(tcs_r, tcs_g, tcs_b, tcs_c);		//Normalización a escala de 0 a 255
			const char* color_str = "Indefinido";

			if(modo_seleccionado == 0){		//MODO DISCO (0: Rojo, 1: Verde, 2: Azul, 3: Blanco, 4: Negro)

				uint8_t sector = COLOR_Disc(tcs_r, tcs_g, tcs_b, tcs_c);

				/* Mueve el servomotor al sector correspondiente (0 a 4) */
				SERVO_SetSector(sector);

				if(sector == 0)      color_str = "Rojo";
				else if(sector == 1) color_str = "Verde";
				else if(sector == 2) color_str = "Azul";
				else if(sector == 3) color_str = "Blanco";
				else if(sector == 4) color_str = "Negro";

			}

			else if(modo_seleccionado == 1){		//MODO BRILLANTES

				color_str = COLOR_Clas_Bri(rgb255);

			}

			else{		//MODO NEUTROS

				color_str = COLOR_Clas_Neu(tcs_c);

			}

			/*Actualización de la pantalla LCD*/
			LCD_Clear();
			LCD_SetCursor(0, 0);
			LCD_Print("COLOR DETECTADO:");
			LCD_SetCursor(1, 0);
			LCD_Print((char*)color_str);

			/*Mensaje trasmitido a la terminal*/
			snprintf(uart_buf, sizeof(uart_buf),
			 "------------------------------\r\n"
			 " Lectura realizada\r\n"
			 "  R = %-3d  G = %-3d  B = %-3d\r\n"
			 "  Color = %s\r\n"
			 "------------------------------\r\n",
			 rgb255.r, rgb255.g, rgb255.b, color_str);

			HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 200);

			HAL_Delay(1500);		//Pausa antes de retornar a estado de espera

			refrescar_pantalla = 1;
			estado_actual = ESTADO_ESPERA;

			break;
		}
	}
}


/*
 * HAL_TIM_PeriodElapsedCallback
 * Llamado automáticamente por HAL_TIM_IRQHandler() cada vez que un evento de actualización del timer se dispara
 * Es compartido por todos los timers (Siempre verifica htim->Instance)
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

    if(htim->Instance == TIM11){

        HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_1);

    }
}


/*
 * HAL_UART_RxCpltCallback
 * Exclusivamente levanta la bandera al recibir datos por USART2
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

	if(huart->Instance == USART2){

		usart_done = 1;

	}

}


