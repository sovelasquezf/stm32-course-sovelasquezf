/*
 * Configuraciones.c
 *
 *  Created on: Jul 31, 2026
 *      Author: savf
 */
#include <stm32f4xx_hal.h>


TIM_HandleTypeDef htim2;				//TIM2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim3;				//TIM3 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

TIM_HandleTypeDef htim11;				//TIM4 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

UART_HandleTypeDef huart2 = {0};		//USART2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

I2C_HandleTypeDef hi2c1;				//I2C1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el


/*Variables*/
volatile uint8_t rx_data = 0;				//Almacena el caracter recibido en la recepción
volatile uint32_t servo_pulse = 1500;		//Comienza en el punto medio del servo


/*Prototipo de funciones*/
void SystemClock_Config(void);
void gpio_Init(void);
void tim2_encoder_Init(void);
void tim3_servo_Init(void);
void tim11_led_ok_Init(void);
void usart2_Init(void);
void i2c1_Init(void);


/*Funciones*/
/*
 * SystemClock_Config
 * Usa el oscilador interno HSI a 16 MHz
 * Sin PLL — configuración de reloj más simple posible
 */
void SystemClock_Config(void){

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
 * Configura PH1 como salida Push-Pull (Led D2 de la tarjeta nucleo)
 */
void gpio_Init(void){

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
 * tim2_encoder_Init
 * Configura el TIM2 (32 bits) en modo Encoder
 * Utiliza los canales 1, 2 y 3 para las entradas CLK (PA0), DT (PA1) y SW (PA4)
 */
void tim2_encoder_Init(void){

	/*Configuración de pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_encoder = {0};
	GPIO_InitTypeDef GPIO_Init_encoder_sw = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA0 y PA1*/
	GPIO_Init_encoder.Pin 		= GPIO_PIN_0 |
							  	  GPIO_PIN_1;
	GPIO_Init_encoder.Mode 		= GPIO_MODE_AF_PP;			//Establece los pines en modo función alternativa
	GPIO_Init_encoder.Pull 		= GPIO_PULLUP;				//Activa resistencia Pull-Up interna para evitar estados flotantes
	GPIO_Init_encoder.Speed 	= GPIO_SPEED_FREQ_HIGH;		//Configuración de velocidad como alta
	GPIO_Init_encoder.Alternate = GPIO_AF1_TIM2;			//Función alternativa correspondiente a AF1 en TIM2

	/*Configuración general de los pines: PA4 (Interrupción EXTI por Flanco de Bajada)*/
	GPIO_Init_encoder_sw.Pin  = GPIO_PIN_4;
	GPIO_Init_encoder_sw.Mode = GPIO_MODE_IT_FALLING;		//Establece el en modo de entrada
	GPIO_Init_encoder_sw.Pull = GPIO_PULLUP;				//Activa resistencia Pull-Up (0 lógico al presionar y 1 cuando no)

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_GPIO_Init(GPIOA, &GPIO_Init_encoder);
	HAL_GPIO_Init(GPIOA, &GPIO_Init_encoder_sw);

	/*Habilitar interrupción solo para el botón (EXTI4)*/
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

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
 * init_TIM3
 * Timer para el PWM del servo
 * PA6 -> AF2 (TIM3_CH1)
 */
void tim3_servo_Init(void){

	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_servo = {0};

	/*Habilitar reloj de TIM3 en el bus APB1*/
    __HAL_RCC_TIM3_CLK_ENABLE();

    /*Habilitar reloj de GPIOA en el bus AHB1*/
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configuración del pin PA6*/
    GPIO_Init_servo.Pin 	  = GPIO_PIN_6;
    GPIO_Init_servo.Mode 	  = GPIO_MODE_AF_PP;				//Establece el pin en modo salida Push-Pull
    GPIO_Init_servo.Pull 	  = GPIO_NOPULL;					//Desactiva resistencias de Pull-Up o Pull-Down
    GPIO_Init_servo.Speed 	  = GPIO_SPEED_FREQ_VERY_HIGH;		//Configuración de velocidad como muy alta
    GPIO_Init_servo.Alternate = GPIO_AF2_TIM3;					//Función alternativa correspondiente a AF2 en TIM3

    /*Cargar la configuracion en los registros FSR del MCU*/
    HAL_GPIO_Init(GPIOA, &GPIO_Init_servo);

    /*Configuración general del TIM3*/
    htim3.Instance				 = TIM3;
    htim3.Init.Prescaler		 = 16 - 1;								//Configurando el Prescaler a 1 us (16 MHz / 16 Hz = 1 MHz)
    htim3.Init.Period			 = 20000 - 1;              				//Periodo de 1 us * 20000 = 20000 us = 20 ms (Frecuencia = 50 Hz)
    htim3.Init.CounterMode 		 = TIM_COUNTERMODE_UP;					//Conteo ascendente
    htim3.Init.ClockDivision 	 = TIM_CLOCKDIVISION_DIV1;				//División en 1 = 20000 us
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;		//Deshabilita la precarga automática

    /*Cargar la configuracion en los registros FSR del MCU*/
    HAL_TIM_PWM_Init(&htim3);

    /*Inicialización de estructuras*/
    TIM_OC_InitTypeDef sConfigOC = {0};

    /*Configuración general del PWM1*/
    sConfigOC.OCMode 	 = TIM_OCMODE_PWM1;			//Establece el modo PWM1 (Salida en alto mientras CNT < CCR)
    sConfigOC.Pulse		 = servo_pulse;             //EL ancho del pulso se inicializa en el punto medio (1500 ticks = 1.5 ms / 90°)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;  	//Polaridad baja debido al uso de optoacoplador que invierte la señal
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;		//Evita fallos en el ciclo de trabajo (Sólo cambiará su estado después de una comparación válida entre el CNT y CCR)

    /*Cargar la configuracion en los registros FSR del MCU*/
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

}


/*
 * tim11_led_ok_Init
 * Configura TIM11 para generar un evento de actualización cada 250 ms
 */
void tim11_led_ok_Init(void){

	/*Configuración del TIM11*/
	/*Habilitar reloj de TIM11 en el bus APB2*/
	__HAL_RCC_TIM11_CLK_ENABLE();

	/*Configuración general del TIM11*/
	htim11.Instance				  = TIM11;
	htim11.Init.Prescaler		  = 16000 - 1;							//Configurando el Prescaler a 1 ms (16 MHz / 16 kHz = 1 kHz)
	htim11.Init.CounterMode 	  = TIM_COUNTERMODE_UP;					//Conteo ascendente
	htim11.Init.Period 			  = 250 - 1;							//Periodo de 1 ms * 250 = 250 ms
	htim11.Init.ClockDivision 	  = TIM_CLOCKDIVISION_DIV1;				//División en 1 = 250 ms
	htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;		//Deshabilita la precarga automática

	/*Cargando la configuración en los registros FSR del MCU*/
	HAL_TIM_Base_Init(&htim11);

	/*Registrando la interrupción en el NVIC para la recepción*/
	HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

	/*Inicialización del TIM11 en modo de interrupción Update Event Interrupt (UEI)*/
	HAL_TIM_Base_Start_IT(&htim11);

}


/*
 * usart2_Init
 * Configuración de pines físicos como función alternada para la comunicación serial
 * 19200 8N1
 */
void usart2_Init(void){

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
	HAL_UART_Receive_IT(&huart2, (uint8_t *) &rx_data, 1);		//El dato se guarda en la variable rx_data de a byte

}


/*
 * i2c1_Init
 * Configura los pines PB8 (SCL) y PB9 (SDA) para el uso de I2C
 */
void i2c1_Init(void){

	/*Configuración de pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_i2c = {0};

	/*Habilitar reloj de GPIOB en el bus AHB1*/
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configuración general de los pines: PB8 y PB9*/
	GPIO_Init_i2c.Pin		= GPIO_PIN_8 |
							  GPIO_PIN_9;
	GPIO_Init_i2c.Mode 	 	= GPIO_MODE_AF_OD;					//Establece los pines en modo alternativo Open-Drain (Requerido para I2C)
	GPIO_Init_i2c.Pull	 	= GPIO_PULLUP;						//Activa resisetncia Pull-Up interna para evitar estados flotantes
	GPIO_Init_i2c.Speed	 	= GPIO_SPEED_FREQ_VERY_HIGH;		//Configuración de velocidad muy alta para flancos de subida y bajada limpios
	GPIO_Init_i2c.Alternate = GPIO_AF4_I2C1;					//Función alternativa correspondiente a AF4 en I2C1

    /*Cargar la configuracion en los registros FSR del MCU*/
    HAL_GPIO_Init(GPIOB, &GPIO_Init_i2c);

    /*Configuración de SCL y SDA en I2C1*/
	/*Habilitar reloj de I2C1 en el bus APB1*/
	__HAL_RCC_I2C1_CLK_ENABLE();

    /*Configuración general del I2C1*/
	hi2c1.Instance	   		   = I2C1;
	hi2c1.Init.ClockSpeed	   = 100000;						//Velocidad de transmisión estándar a 100 kHz (Standard Mode)
	hi2c1.Init.DutyCycle	   = I2C_DUTYCYCLE_2;				//Relación de ciclo de trabajo 2:1 (1/3 en alto y 2/3 en bajo)
	hi2c1.Init.OwnAddress1	   = 0;								//Dirección propia del microcontrolador (Es 0 actuando como maestro)
	hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;		//Direcciones del bus de 7 bits
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;		//Desactiva direccionamiento dual
	hi2c1.Init.OwnAddress2	   = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;		//Desactiva el broadcast (Llamada general)
	hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;			//Permite Clock Stretching (Estiramiento del reloj) para dar tiempo de procesamiento al sensor

	/*Inicialización del I2C*/
	HAL_I2C_Init(&hi2c1);

}
