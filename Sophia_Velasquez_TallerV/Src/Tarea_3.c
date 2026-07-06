/*
 * Tarea_3.c
 *
 *  Created on: Jul 3, 2026
 *      Author: Sophia Alejandra Velasquez Fuentes - sovelasquezf@unal.edu.co
 *      Brief: Contador con fotocompuertas
 *
 *  Distribución de los Timers - Pines
 *
 *  - TIM1 - AF01 (PWM)
 *  	CH2 -> PA9: Color verde
 *  	CH3 -> PA10: Color Rojo
 *  	CH4 -> PA11: Color Azul
 *
 *  - TIM2 - AF01
 *    (Encoder - Verde)
 *  	CH1 -> PA0: DT
 *  	CH2 -> PA1: CLK
 *
 *    (Comunicación Serial - Rojo)
 *  	CH3 -> PA2: USART2_TX (Transmisión)
 *  	CH4 -> PA3: USART2_RX (Recepción)
 *
 *  - TIM3 - AF02 (Potenciómetro - Azul)
 *  	CH1 -> PA6: ADC1_6 (Conversión analogo-digital)
 *
 *  - TIM4 (Led OK)
 * 		PH1: Pin asociado a D2 en la board táctica
 *
 * 	- Extra - AF00:
 * 		PA8: MC0_1
 *
 *
 *
 */

#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <string.h>

TIM_HandleTypeDef htim1;			//TIM1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim2;			//TIM2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim3;			//TIM3 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim4;			//TIM4 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

ADC_HandleTypeDef hacd1 = {0};		//ADC1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

USART_HandleTypeDef husart2 = {0};	//USART2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el


/*Variables*/


/*Prototipo de funciones privadas*/
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim1_pwm_Init(void);
static void tim2_encoder_Init(void);
static void tim3_adc_Init(void);
static void tim4_led_ok_Init(void);
static void usart2_Init(void);
static void adc_Init(void);
static void mc01_Init(void);


int main(void){

	HAL_Init();
	SystemClock_Config();


	while(1){

	}

}


/*Funciones privadas*/

/*
 * SystemClock_Config
 * Usa el oscilador interno HSI a 16 MHz
 * Sin PLL — configuración de reloj más simple posible
 */
static void SystemClock_Config(void){

	/*Inicialización de estructuras*/
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*HSI ya está encendido al resetear (Confirmar y usarlo)*/
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;			//Oscilador Interno de Alta Velocidad (HSI)
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;						//Encendido del HSI
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;		//Calibración por defecto de fábrica para el HSI
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;					//Desactivación del PLL para trabajar a 16 MHz

    /*Cargando la configuración en los registros FSR del MCU*/
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /*Seleccionar HSI como SYSCLK (Todos los divisores de bus en 1)*/
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_SYSCLK |	//Configuración simultánea de los relojes
                                       RCC_CLOCKTYPE_HCLK   |
                                       RCC_CLOCKTYPE_PCLK1  |
                                       RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;	//Reloj del sistema (SYSCLK) = 16 MHz
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   		//Divisor del AHB (HCLK) en 1 = 16 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;     		//Divisor del APB1 (PCLK1) en 1 = 16 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     		//Divisor del APB2 (PCLK2) en 1 = 16 MHz

    /*FLASH_LATENCY_0 = cero wait states, correcto para 16 MHz*/
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

}


static void gpio_Init(void){



}


static void tim1_pwm_Init(void){



}


static void tim2_encoder_Init(void){



}


static void tim3_adc_Init(void){



}


static void tim4_led_ok_Init(void){



}


static void usart2_Init(void){



}


static void adc_Init(void){



}


static void mc01_Init(void){



}






