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
 */

#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <string.h>

TIM_HandleTypeDef htim1;			//TIM1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim2;			//TIM2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim3;			//TIM3 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim4;			//TIM4 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

ADC_HandleTypeDef hadc1 = {0};		//ADC1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

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
	gpio_Init();

	tim3_adc_Init();
	adc_Init();




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


/*
 * gpio_Init
 * Configura PH1 como salida push-pull (LED D2 de la tarjeta nucleo)
 */
static void gpio_Init(void){

	/*Inicialización de estructuras*/
    GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Habilitar reloj de GPIOH en el bus AHB1 (Equivalente bare-metal: RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN)*/
	__HAL_RCC_GPIOH_CLK_ENABLE();

    /*Configuración PH1*/
    GPIO_InitStruct.Pin   = GPIO_PIN_1;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    /*Cargando la configuración en los registros FSR del MCU*/
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    __NOP();

}



static void tim1_pwm_Init(void){



}


static void tim2_encoder_Init(void){



}

/*
 * Configuración del TIM3 a 20 ms para disparar el ADC usando la señal TGRO
 */
static void tim3_adc_Init(void){

	/*Habilitar reloj de TIM3 en el bus APB1*/
	__HAL_RCC_TIM3_CLK_ENABLE();

	/*Configuración base del TIM3*/

	htim3.Instance				 = TIM3;
	htim3.Init.Prescaler		 = 16000 - 1;						//Configurando el Prescaler a 1 ms (16 MHz / 16 kHz = 1 kHz)
	htim3.Init.CounterMode 		 = TIM_COUNTERMODE_UP;				//Conteo ascendente
	htim3.Init.Period 			 = 20 - 1;							//Periodo de 1 ms * 20 = 20 ms
	htim3.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;			//División en 1 = 20 ms
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	/*Cargando la configuración en los registros FSR del MCU*/
	HAL_TIM_Base_Init(&htim3);

	/*Configuración del trigger (TGRO)*/

	/*Inicialización de estructuras*/
	TIM_MasterConfigTypeDef MasterConfig_TGRO = {0};

	/*Configuración general del TGRO*/
	MasterConfig_TGRO.MasterOutputTrigger = TIM_TRGO_UPDATE;				//Genera un evento de actualización cada 20 ms
	MasterConfig_TGRO.MasterSlaveMode 	  = TIM_MASTERSLAVEMODE_DISABLE;

	/*Cargando la configuración en los registros FSR del MCU*/
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &MasterConfig_TGRO);

	/*Inicialización del TIM3*/
	HAL_TIM_Base_Start(&htim3);

}


static void tim4_led_ok_Init(void){



}


static void usart2_Init(void){



}

/*
 * adc_Init
 * Configuración del ADC
 */
static void adc_Init(void){

	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_adc_ch6 = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración de PA6*/
	GPIO_Init_adc_ch6.Pin  = GPIO_PIN_6;
	GPIO_Init_adc_ch6.Mode = GPIO_MODE_ANALOG;
	GPIO_Init_adc_ch6.Pull = GPIO_NOPULL;

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_GPIO_Init(GPIOA, &GPIO_Init_adc_ch6);
	__NOP();

	/*Habilitar reloj de ADC en el bus APB2*/
	__HAL_RCC_ADC1_CLK_ENABLE();

	/*Configuración general del ADC*/
	hadc1.Instance					 = ADC1;
	hadc1.Init.ClockPrescaler		 = ADC_CLOCK_SYNC_PCLK_DIV2;			//División 2: 16 MHz / 2 = 8 MHz
	hadc1.Init.Resolution			 = ADC_RESOLUTION_12B;					//Resolución 12 bit (4096 divisiones)
	hadc1.Init.DataAlign			 = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode			 = DISABLE;								//Deshabilitado por ser el unico canal de conversión ADC
	hadc1.Init.EOCSelection			 = ADC_EOC_SINGLE_CONV;					//Conversión finaliza al terminal el canal
	hadc1.Init.ContinuousConvMode	 = DISABLE;
	hadc1.Init.NbrOfConversion 		 = 1;									//Sólo hay un canal haciendo conversión ADC
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv		 = ADC_EXTERNALTRIGCONV_T3_TRGO;		//Se usa el TGRO del TIM3 como trigger para la conversión
	hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;		//Se dispara el ADC cada vez que se detecta un flanco de subida
	hadc1.Init.DMAContinuousRequests = DISABLE;

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_ADC_Init(&hadc1);

	/*Configuración del canal específico*/

	/*Inicialización de estructuras*/
	ADC_ChannelConfTypeDef adc_ch6 = {0};

	/*Configuración general del canal*/
	adc_ch6.Channel		 = ADC_CHANNEL_6;
	adc_ch6.Rank		 = 1;							//Primera y única conversión a hacer
	adc_ch6.SamplingTime = ADC_SAMPLETIME_56CYCLES;		//Cantidad de ciclos a pasar para hacer el muestreo (carga del capacitor)
	adc_ch6.Offset		 = 0;

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_ADC_ConfigChannel(&hadc1, &adc_ch6);

	/*Registrar la interrupción en el NVIC*/
	HAL_NVIC_EnableIRQ(ADC_IRQn);

	/*Inicialización del ADC1*/
	HAL_ADC_Start_IT(&hadc1);

}


static void mc01_Init(void){



}






