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
 *  - TIM2
 *    (Encoder - Verde) - AF01
 *  	CH1 -> PA0: DT
 *  	CH2 -> PA1: CLK
 *
 *    (Comunicación Serial - Rojo) - AF07
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
 * 		PA8: MCO_1
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

UART_HandleTypeDef huart2 = {0};	//USART2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el


/*Variables*/
uint8_t rx_data = 0;

/*Prototipo de funciones privadas*/
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim1_pwm_Init(void);
static void tim2_encoder_Init(void);
static void tim3_adc_Init(void);
static void tim4_led_ok_Init(void);
static void usart2_Init(void);
static void adc_Init(void);
static void mco1_Init(void);


int main(void){

	HAL_Init();
	SystemClock_Config();
	gpio_Init();
	tim4_led_ok_Init();

	tim3_adc_Init();
	adc_Init();
	tim1_pwm_Init();
	tim2_encoder_Init();
	usart2_Init();
	mco1_Init();

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

	/*Habilitar reloj de GPIOH en el bus AHB1*/
	__HAL_RCC_GPIOH_CLK_ENABLE();

    /*Configuración PH1*/
    GPIO_InitStruct.Pin   = GPIO_PIN_1;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;		//Establece el pin en modo salida Push-Pull
    GPIO_InitStruct.Pull  = GPIO_NOPULL;				//Desactiva resistencias de Pull-Up o Pull-Down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;		//Configuración de velocidad como baja

    /*Cargando la configuración en los registros FSR del MCU*/
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

}


/*
 * tim1_pwm_Init
 * Configura el TIM1 en modo PWM para controlar los canales 2, 3 y 4
 * Frecuencia 2.5 kHz
 */
static void tim1_pwm_Init(void){

	/*Configuración de pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_pwm = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA9, PA10, PA11*/
	GPIO_Init_pwm.Pin		= GPIO_PIN_9  |
							  GPIO_PIN_10 |
							  GPIO_PIN_11;
	GPIO_Init_pwm.Mode		= GPIO_MODE_AF_PP;			//Establece los pines en modo función alternativa
	GPIO_Init_pwm.Pull		= GPIO_NOPULL;				//Desactiva resistencias de Pull-Up o Pull-Down
	GPIO_Init_pwm.Speed		= GPIO_SPEED_FREQ_HIGH;		//Configuración de velocidad como alta
	GPIO_Init_pwm.Alternate = GPIO_AF1_TIM1;			//Función alernativa correspondiente a AF1 en TIM1

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_GPIO_Init(GPIOA, &GPIO_Init_pwm);

	/*Configuración del TIM1*/
	/*Habilitar reloj de TIM1 en el bus APB2*/
	__HAL_RCC_TIM1_CLK_ENABLE();

	/*Configuración general del TIM1*/
	htim1.Instance				 = TIM1;
	htim1.Init.Prescaler		 = 16 - 1;								//Configurando el Prescaler a 1 us (16 MHz / 16 = 1 MHz)
	htim1.Init.CounterMode		 = TIM_COUNTERMODE_UP;					//Conteo ascendente
	htim1.Init.Period 			 = 400 - 1;								//Periodo de 1 us * 400 = 400 us (2.5 kHz)
	htim1.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;				//División en 1 = 2.5 kHz
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;		//Habilita la precarga automática

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_TIM_PWM_Init(&htim1);

	/*Configuración de canales del TIM1: CH2, CH3 y CH4*/
	/*Inicialización de estructuras*/
	TIM_OC_InitTypeDef ConfigOC =  {0};

	/*Configuración general de los canales*/
	ConfigOC.OCMode 	= TIM_OCMODE_PWM1;			//Establece el modo PWM1 (Salida en alto mientras CNT < CCR)
	ConfigOC.Pulse 		= 0;						//Inicializa el CCR en 0%
	ConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;		//Configura polaridad alta (En alto hay un 1 lógico)
	ConfigOC.OCFastMode = TIM_OCFAST_DISABLE;		//Evita fallos en el ciclo de trabajo (Sólo cambiará su estado después de una comparación válida entre el CNT y CCR)

	/*Cargar la configuracion en los registros FSR del MCU (Para cada canal) */
	HAL_TIM_PWM_ConfigChannel(&htim1, &ConfigOC, TIM_CHANNEL_2);
	HAL_TIM_PWM_ConfigChannel(&htim1, &ConfigOC, TIM_CHANNEL_3);
	HAL_TIM_PWM_ConfigChannel(&htim1, &ConfigOC, TIM_CHANNEL_4);

	/*Inicialiación de cada canal */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

}


/*
 * tim2_encoder_Init
 * Configura el TIM2 (32 bits) en modo Encoder
 * Utiliza los canales 1 y 2 para las entradas DT (PA0) y CLK (PA1)
 */
static void tim2_encoder_Init(void){

	/*Configuración de pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_encoder = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA0 y PA1*/
	GPIO_Init_encoder.Pin 		= GPIO_PIN_0 |
							  	  GPIO_PIN_1;
	GPIO_Init_encoder.Mode 		= GPIO_MODE_AF_PP;			//Establece los pines en modo función alternativa
	GPIO_Init_encoder.Pull 		= GPIO_PULLUP;				//Activa resistencia Pull-Up interna para evitar estados flotantes
	GPIO_Init_encoder.Speed 	= GPIO_SPEED_FREQ_HIGH;		//Configuración de velocidad como alta
	GPIO_Init_encoder.Alternate = GPIO_AF1_TIM2;			//Función alternativa correspondiente a AF1 en TIM2

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_GPIO_Init(GPIOA, &GPIO_Init_encoder);

	/*Configuración de canales del TIM2: CH1 y CH2*/
	/*Inicialización de estructuras*/
	TIM_Encoder_InitTypeDef Config_encoder = {0};

	/*Habilitar reloj de TIM2 en el bus APB1*/
	__HAL_RCC_TIM2_CLK_ENABLE();

	/*Configuración general del TIM2*/
	htim2.Instance				 = TIM2;
	htim2.Init.Prescaler		 = 0;									//No se va a hacer división a los pulsos del encoder
	htim2.Init.CounterMode		 = TIM_COUNTERMODE_UP;					//El conteo depende del sentido (CW o CCW)
	htim2.Init.Period 			 = 65535;								//El conteo máximo es 65535 para guardarse en una variable de 16 bit
	htim2.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;				//División en 1
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;		//Deshabilita la precarga automática

	/*Configuración de los canales*/
	/*Configuración del encoder para usar ambos canales CH1 y CH2*/
	Config_encoder.EncoderMode = TIM_ENCODERMODE_TI12;		//Detecta flancos de subida y bajada en cada canal para calcular el sentido de giro (Cuenta 4 veces por ciclo)

	/*Configuración general para CH1*/
	Config_encoder.IC1Polarity 	= TIM_INPUTCHANNELPOLARITY_RISING;		//Polaridad de entrada con flanco de subida
	Config_encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;				//El input 1 (PA0) se conecta al IC1
	Config_encoder.IC1Prescaler = TIM_ICPSC_DIV1;						//La captura se hace al detectar un flanco sin división
	Config_encoder.IC1Filter 	= 15;									//Se usa el valor máximo del filtro para evitar rebotes

	/*Configuración general para CH2*/
	Config_encoder.IC2Polarity 	= TIM_INPUTCHANNELPOLARITY_RISING;		//Polaridad de entrada con flanco de subida
	Config_encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;				//El input 2 (PA1) se conecta al IC2
	Config_encoder.IC2Prescaler = TIM_ICPSC_DIV1;						//La captura se hace al detectar un flanco sin división
	Config_encoder.IC2Filter 	= 15;									//Se usa el valor máximo del filtro para evitar rebotes

	/*Cargar la configuracion en los registros FSR del MCU*/
	HAL_TIM_Encoder_Init(&htim2, &Config_encoder);

	/*Inicialización de todos los canales*/
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

}

/*
 * tim3_adc_Init
 * Configuración del TIM3 a 20 ms para disparar el ADC usando la señal TGRO
 */
static void tim3_adc_Init(void){

	/*Configuración del TIM3*/
	/*Habilitar reloj de TIM3 en el bus APB1*/
	__HAL_RCC_TIM3_CLK_ENABLE();

	/*Configuración general del TIM3*/
	htim3.Instance				 = TIM3;
	htim3.Init.Prescaler		 = 16000 - 1;							//Configurando el Prescaler a 1 ms (16 MHz / 16 kHz = 1 kHz)
	htim3.Init.CounterMode 		 = TIM_COUNTERMODE_UP;					//Conteo ascendente
	htim3.Init.Period 			 = 20 - 1;								//Periodo de 1 ms * 20 = 20 ms
	htim3.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;				//División en 1 = 20 ms
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;		//Deshabilita la precarga automática

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


/*
 * tim4_led_ok_Init
 * Configura TIM4 para generar un evento de actualización cada 250 ms
 */
static void tim4_led_ok_Init(void){

	/*Configuración del TIM4*/
	/*Habilitar reloj de TIM4 en el bus APB1*/
	__HAL_RCC_TIM4_CLK_ENABLE();

	/*Configuración general del TIM3*/
	htim4.Instance				 = TIM4;
	htim4.Init.Prescaler		 = 16000 - 1;							//Configurando el Prescaler a 1 ms (16 MHz / 16 kHz = 1 kHz)
	htim4.Init.CounterMode 		 = TIM_COUNTERMODE_UP;					//Conteo ascendente
	htim4.Init.Period 			 = 250 - 1;								//Periodo de 1 ms * 250 = 250 ms
	htim4.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;				//División en 1 = 250 ms
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;		//Deshabilita la precarga automática

	/*Cargando la configuración en los registros FSR del MCU*/
	HAL_TIM_Base_Init(&htim4);

	/*Registrando la interrupción en el NVIC para la recepción*/
	HAL_NVIC_EnableIRQ(TIM4_IRQn);

	/*Inicialización del TIM4 en modo de interrupción Update Event Interrupt (UEI)*/
	HAL_TIM_Base_Start_IT(&htim4);

}


/*
 * usart2_Init
 * Configuración de pines físicos como función alternada para la comunicación serial
 * 19200 8N1
 */
static void usart2_Init(void){

	/*Configuración de los pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_Tx = {0};
	GPIO_InitTypeDef GPIO_Init_Rx = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA2 y PA3 */
	GPIO_Init_Tx.Pin	   = GPIO_PIN_2;
	GPIO_Init_Tx.Mode 	   = GPIO_MODE_AF_PP;			//Establece los pines en modo función alternativa
	GPIO_Init_Tx.Pull  	   = GPIO_NOPULL;				//Desactiva resistencias de Pull-Up o Pull-Down
	GPIO_Init_Tx.Speed	   = GPIO_SPEED_FREQ_HIGH;		//Configuración de velocidad como alta
	GPIO_Init_Tx.Alternate = GPIO_AF7_USART2;			//Función alternativa correspondiente a AF7 en USART2

	GPIO_Init_Rx.Pin	   = GPIO_PIN_3;
	GPIO_Init_Rx.Mode 	   = GPIO_MODE_AF_PP;			//Establece los pines en modo función alternativa
	GPIO_Init_Rx.Pull  	   = GPIO_PULLUP;				//Activa resistencia Pull-Up interna para evitar estados flotantes
	GPIO_Init_Rx.Speed	   = GPIO_SPEED_FREQ_HIGH;		//Configuración de velocidad como alta
	GPIO_Init_Rx.Alternate = GPIO_AF7_USART2;			//Función alternativa correspondiente a AF7 en USART2

	/*Cargar la configuración en los registros FSR del MCU*/
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Tx);
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Rx);

	/*Configuración del USART2*/
	/*Habilitar reloj de TIM2 en el bus APB1*/
	__HAL_RCC_USART2_CLK_ENABLE();

	/*Configuración general del USART2*/
	huart2.Instance          = USART2;
	huart2.Init.BaudRate     = 19200;
	huart2.Init.Mode         = UART_MODE_TX_RX;			//Establece modo de transmisión (Tx) y recepción (Rx)
	huart2.Init.Parity       = UART_PARITY_NONE;		//Sin paridad
	huart2.Init.StopBits     = UART_STOPBITS_1;			//1 bit de parada
	huart2.Init.WordLength   = UART_WORDLENGTH_8B;		//8 bits de datos

	/*Cargar la configuración en los registros FSR del MCU*/
	HAL_UART_Init(&huart2);

	/*Registrando la interrupción en el NVIC para la recepción*/
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	/*Habilitar la interrupción para la recepción de datos*/
	HAL_UART_Receive_IT(&huart2, &rx_data, 1);	//El dato se guarda en la variable rx_data de a byte

}


/*
 * adc_Init
 * Configuración del ADC
 */
static void adc_Init(void){

	/*Configuración de PA6*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_adc_ch6 = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general del pin*/
	GPIO_Init_adc_ch6.Pin  = GPIO_PIN_6;
	GPIO_Init_adc_ch6.Mode = GPIO_MODE_ANALOG;		//Configura el pin en modo analógico
	GPIO_Init_adc_ch6.Pull = GPIO_NOPULL;			//Desactiva resistencias de Pull-Up o Pull-Down

	/*Cargar la configuracion en los registros FSR del MCU*/
	HAL_GPIO_Init(GPIOA, &GPIO_Init_adc_ch6);

	/*Configuración del ADC*/
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


/*
 * mco1_Init
 * Configura el MCO1 como salida para leer el HSI en PA8
 */
static void mco1_Init(void){

	/*Configuración de los pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef mco1_Init = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA8*/
	mco1_Init.Pin	    = GPIO_PIN_8;
	mco1_Init.Mode 	    = GPIO_MODE_AF_PP;				//Establece los pines en modo función alternativa
	mco1_Init.Pull  	= GPIO_NOPULL;					//Desactiva resistencias de Pull-Up o Pull-Down
	mco1_Init.Speed	    = GPIO_SPEED_FREQ_VERY_HIGH;	//Configuración de velocidad como muy alta para evitar distorsiones
	mco1_Init.Alternate = GPIO_AF0_MCO;					//Función alternativa correspondiente a AF0 en MCO1

	/*Cargar la configuracion en los registros FSR del MCU*/
	HAL_GPIO_Init(GPIOA, &mco1_Init);

	/*Inicialización del MCO1*/
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);		//Se redirige la señal del Oscilador Interno de Alta Velocidad (HSI) a PA8

}






