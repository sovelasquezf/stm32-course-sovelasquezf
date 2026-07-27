/*
 * Parcial.c
 *
 *  Created on: Jul 21, 2026
 *      Author: Sophia Alejandra Velásquez Fuentes - sovelasquezf@unal.edu.co
 *       Brief:
 */

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_i2c.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define LCD_I2C_ADDR (0x22 << 1)

/*Posiciones de bits según el mapeo del PCF8574*/
#define LCD_RS         0x01		//P0
#define LCD_RW         0x02		//P1 (Se mantiene en 0 para poder escribir)
#define LCD_EN         0x04		//P2 (Comienza a escribir o leer datos)
#define LCD_BACKLIGHT  0x08		//P3 (Se mantiene en 1 para encender)

/*Direcciones base de cada línea (20x4)*/
#define LCD_LINE1  0x00		//Inicio línea A
#define LCD_LINE2  0x40		//Inicio línea B
#define LCD_LINE3  0x14		//Mitad de línea A (0x00 + 20 = 0x14)
#define LCD_LINE4  0x54		//Mitad de línea B (0x40 + 20 = 0x54)

/*Definiciones de RS: distinguen si se envía un comando o un dato al LCD*/
#define LCD_COMANDO  0
#define LCD_DATO     1

/*Contraseña o firma para la inicialización del RTC*/
#define RTC_BACKUP_PSW  1111		//



TIM_HandleTypeDef htim11;				//TIM11 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

ADC_HandleTypeDef hadc1 = {0};			//ADC1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

I2C_HandleTypeDef hi2c1;				//I2C1 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

UART_HandleTypeDef huart2 = {0};		//USART2 handle debe ser global para que stm32f4xx_it.c pueda acceder a el

RTC_HandleTypeDef hrtc;					//RTC handle debe ser global para que stm32f4xx_it.c pueda acceder a el


/*Variables*/
/*Mensaje inicial con las indicaciones para la recepción*/
uint8_t init_Msg[] =
"\r\n"
" Parcial - Sophia Velasquez\r\n"
"--------------------------------------------------------\r\n"
" Caracteres para la recepción:\r\n"
" 'H': Cambia la señal del MCO1 al HSI\r\n"
" 'L': Cambia la señal del MCO1 al LSE\r\n"
" 'P': Cambia la señal del MCO1 al PLL\r\n"
" 'F': Cambia el formato de la hora (Entre 12h y 24h)\r\n"
" 'J': Cambia el formato del Joystick (Normalizado y V)\r\n"
"--------------------------------------------------------\r\n"
"\r\n";

/*Variables usadas para la conversión ADC*/
volatile uint16_t raw_adc_x = 0;		//Almacena el valor de x para la conversión ADC
volatile uint16_t raw_adc_y = 0;		//Almacena el valor de y para la conversión ADC
volatile uint16_t adc_secuence = 0;		//Bandera para la secuencia en la conversión ADC (En 0: Conversión a hacer es x | En 1: Conversión a hacer es y)
volatile uint16_t adc_done = 0;			//Bandera para la conversión ADC

/*Variable usadas para al comunicación serial*/
volatile uint8_t usart_done = 0;		//Bandera para la comunicación serial
volatile uint8_t rx_data = 0;			//Almacena el caracter recibido en la recepción

/*Variable usadas para comparar el esatdo actual del Joystick*/
volatile uint16_t actual_x = 0;   //Ultimo valor de X mostrado (para comparar cambios significativos)
volatile uint16_t actual_y = 0;   //Ultimo valor de Y mostrado (para comparar cambios significativos)

/*Variables usadas para la transmisión del display*/
uint8_t lcd_backlight = LCD_BACKLIGHT;		//Controla el estado de la luz de fondo del LCD

volatile uint8_t  rtc_formato = 0;   				//En 0: 24h | En 1: 12h
volatile uint8_t  rtc_ultimo_segundo = 60;
volatile uint8_t  joystick_formato = 0;				//En 0: Voltaje en V | En 1: vector normalizado
char* mco_actual = "HSI";   	//Texto de la señal activa en MCO1

RTC_TimeTypeDef hora_actual = {0};
RTC_DateTypeDef fecha_actual = {0};

/*Variables de Control de Tiempo (Timers de la FSM)*/
uint32_t last_adc = 0;


/*Mensaje completo donde se muestran los valores actuales del equipo a medida que se van actualizando*/
uint8_t msg_buffer[256];

/*Lineas a mostrar en el display*/
char lcd_linea0[21] = "Fecha: --/--/--";		//Buffer para la fila 0
char lcd_linea1[21] = "Hora: --:--:--";			//Buffer para la fila 1
char lcd_linea2[21] = "X:---- Y:----";			//Buffer para la fila 2
char lcd_linea3[21] = "MCO1: ---";				//Buffer para la fila 3

// Bandera global para saber si debemos actualizar la pantalla
volatile uint8_t update_display_flag = 0;

/*Implementación FSM*/
typedef enum{
	STATE_IDLE,             // Director de tráfico (espera eventos)
	STATE_READ_DATA,
	STATE_UPDATE_DISPLAY    // ÚNICO lugar donde se escribe en la LCD (evita parpadeos)

} FSM_STATE;

FSM_STATE estado_actual = STATE_IDLE;


/*Prototipo de funciones privadas*/
static void SystemClock_Config(void);
static void gpio_Init(void);
static void tim11_led_ok_Init(void);
static void adc_Init(void);
static uint8_t adc_process(void);
static void i2c1_display_Init(void);
static void LCD_SendNibble(uint8_t nibble, uint8_t rs);
static void LCD_SendCommand(uint8_t comando);
static void LCD_Init(void);
static void LCD_SetCursor(uint8_t fila, uint8_t columna);
static void LCD_Print(char *texto);
static void LCD_Refresh(void);
static void usart2_Init(void);
static void command_process(void);
static void rtc_Init(void);
static void rtc_Initial_Setting(void);
static uint8_t rtc_process(void);
static void update_data(void);
static void mco1_Init(void);


int main(void){

	/*Inicialización de los bloques independientes*/
	HAL_Init();					//Inicializa HAL: SysTick, caché, agrupación de prioridades
	SystemClock_Config();		//Configura el árbol de relojes: HSI a 16 MHz y se activa el PLL para trabajar con el procesador a 100 MHz
	gpio_Init();
	tim11_led_ok_Init();
	adc_Init();
	i2c1_display_Init();
	LCD_Init();
	usart2_Init();
	rtc_Init();
	rtc_Initial_Setting();
	mco1_Init();

	/*Transmisión del mensaje inicial (Instrucción con los caracteres específicos a usar)*/
	HAL_UART_Transmit(&huart2, (uint8_t *) init_Msg, strlen((char *) init_Msg), 300);

	HAL_ADC_Start_IT(&hadc1);

	// 3. Forzar una primera lectura para que la pantalla no inicie en blanco
	update_display_flag = 1;

	/* Inicializar timers */
	last_adc = HAL_GetTick();

	while(1){

		switch(estado_actual){

		case STATE_IDLE:

			if(usart_done == 1){

		        estado_actual = STATE_READ_DATA;

		    }

			else if (HAL_GetTick() - last_adc >= 250){

				last_adc = HAL_GetTick();

				HAL_ADC_Start_IT(&hadc1);

			}


			else if (adc_done == 1) {

				estado_actual = STATE_READ_DATA;

			}

			break;

		case STATE_READ_DATA:

			if(usart_done == 1){

				command_process();

				usart_done = 0;             // ¡Muy importante! Bajamos la bandera
				update_display_flag = 1;    // Hubo un cambio, pedimos actualizar pantalla

				HAL_UART_Receive_IT(&huart2, (uint8_t *) &rx_data, 1);

			}

			if(adc_done == 1){

				if(adc_process() == 1){

					update_display_flag = 1; // FSM levanta la bandera

				}

				adc_done = 0;

			}

			if(rtc_process() == 1){

				update_display_flag = 1;     // FSM levanta la bandera

			}

			if (update_display_flag == 1) {

				estado_actual = STATE_UPDATE_DISPLAY;

			}

			else {

				estado_actual = STATE_IDLE;

			}

			break;

		case STATE_UPDATE_DISPLAY:

		// 1. Llamamos a la superfunción que da formato a X/Y/MCO1, refresca la LCD y envía el UART
			update_data();

			// 2. Bajamos la bandera porque ya cumplimos la tarea
			update_display_flag = 0;

			// 3. Volvemos al estado de reposo a esperar el siguiente evento
			estado_actual = STATE_IDLE;

			break;



		        } // Fin del switch



	}

}


/*Funciones privadas*/

/*
 * SystemClock_Config
 * Usa el oscilador interno HSI a 16 MHz
 * Se configura el PLL a 100 MHz
 */
static void SystemClock_Config(void){

	/*Inicialización de estructuras*/
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /*Habilitar el reloj del periferico de power control*/
    __HAL_RCC_PWR_CLK_ENABLE();

	/*Subir el regulador a Scale 1 (Condición para que la frecuencia del HCLK máxima sea 100 MHz según el datasheet)*/
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/*Habilitar acceso al dominio de backup (requerido para tocar el LSE y el RTC)*/
	HAL_PWR_EnableBkUpAccess();

    /*HSI ya está encendido al resetear (Confirmar y usarlo) y del LSE para ser usado en MCO1 y el RTC*/
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI |		//Oscilador Interno de Alta Velocidad (HSI)
    										RCC_OSCILLATORTYPE_LSE;			//Oscilador Externo de Baja Velocidad (LSE)
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;						//Encendido del HSI
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;		//Calibración por defecto de fábrica para el HSI
    RCC_OscInitStruct.LSEState  	 	  = RCC_LSE_ON;						//Encendido del LSE

    /*Configuración del PLL a 100 MHz*/
    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;				//Activación del PLL
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;		//Selecciona el HSI como fuente para el PLL
	RCC_OscInitStruct.PLL.PLLM      = 16;						//Divisor en 16 = 1 MHz
	RCC_OscInitStruct.PLL.PLLN      = 200;						//Multiplicador en 200 = 200 MHz
	RCC_OscInitStruct.PLL.PLLP      = RCC_PLLP_DIV2;			//Divisor en 2 = 100 MHz

    /*Cargando la configuración en los registros FSR del MCU*/
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /*Seleccionar PLL como SYSCLK*/
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_SYSCLK |		//Configuración simultánea de los relojes
                                       RCC_CLOCKTYPE_HCLK   |
                                       RCC_CLOCKTYPE_PCLK1  |
                                       RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;		//Reloj del sistema (SYSCLK) = 100 MHz
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   			//Divisor del AHB (HCLK) en 1 = 100 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     			//Divisor del APB1 (PCLK1) en 2 = 50 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     			//Divisor del APB2 (PCLK2) en 1 = 100 MHz

    /*FLASH_LATENCY_3 = three wait states, correcto para 100 MHz*/
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);

}


/*
 * gpio_Init
 * Configura PH1 como salida Push-Pull (Led D2 de la tarjeta nucleo)
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
 * tim11_led_ok_Init
 * Configura TIM11 para generar un evento de actualización cada 250 ms
 */
static void tim11_led_ok_Init(void){

	/*Configuración del TIM11*/
	/*Habilitar reloj de TIM11 en el bus APB2*/
	__HAL_RCC_TIM11_CLK_ENABLE();

	/*Configuración general del TIM11*/
	htim11.Instance				  = TIM11;
	htim11.Init.Prescaler		  = 50000 - 1;							//Configurando el Prescaler a 0.5 ms (100 MHz / 50 kHz = 2 kHz)
	htim11.Init.CounterMode 	  = TIM_COUNTERMODE_UP;					//Conteo ascendente
	htim11.Init.Period 			  = 500 - 1;							//Periodo de 0.5 ms * 500 = 250 ms
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
 * adc_Init
 * Configuración del ADC para PA5(CH5) y PA6(CH6)
 */
static void adc_Init(void){

	/*Configuración de PA5 y PA6*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_adc_ch5_ch6 = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general del pin*/
	GPIO_Init_adc_ch5_ch6.Pin  = GPIO_PIN_5 |
								 GPIO_PIN_6;
	GPIO_Init_adc_ch5_ch6.Mode = GPIO_MODE_ANALOG;		//Configura el pin en modo analógico
	GPIO_Init_adc_ch5_ch6.Pull = GPIO_NOPULL;			//Desactiva resistencias de Pull-Up o Pull-Down

	/*Cargar la configuracion en los registros FSR del MCU*/
	HAL_GPIO_Init(GPIOA, &GPIO_Init_adc_ch5_ch6);

	/*Configuración del ADC*/
	/*Habilitar reloj de ADC en el bus APB2*/
	__HAL_RCC_ADC1_CLK_ENABLE();

	/*Configuración general del ADC*/
	hadc1.Instance					 = ADC1;
	hadc1.Init.ClockPrescaler		 = ADC_CLOCK_SYNC_PCLK_DIV8;		//División 8: 100 MHz / 8 = 12.5 MHz
	hadc1.Init.Resolution			 = ADC_RESOLUTION_12B;				//Resolución 12 bit (4096 divisiones)
	hadc1.Init.DataAlign			 = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode			 = ENABLE;							//Habilitado por el eso de más de un solo canal de conversión ADC
	hadc1.Init.EOCSelection			 = ADC_EOC_SINGLE_CONV;				//Conversión finaliza al terminal el canal
	hadc1.Init.ContinuousConvMode	 = DISABLE;
	hadc1.Init.NbrOfConversion 		 = 2;								//Hay dos canales haciendo conversión ADC
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv		 = ADC_SOFTWARE_START;				//
	hadc1.Init.DMAContinuousRequests = DISABLE;

	/*Cargar la configuracion en los registros FSR del MCU */
	HAL_ADC_Init(&hadc1);

	/*Configuración de los canales 5 y 6*/
	/*Inicialización de estructuras*/
	ADC_ChannelConfTypeDef adc_ch5 = {0};
	ADC_ChannelConfTypeDef adc_ch6 = {0};

	/*Configuración general del canal 5*/
	adc_ch5.Channel		 = ADC_CHANNEL_5;
	adc_ch5.Rank		 = 1;							//Primera  conversión a hacer
	adc_ch5.SamplingTime = ADC_SAMPLETIME_84CYCLES;		//Cantidad de ciclos a pasar para hacer el muestreo (carga del capacitor)
	adc_ch5.Offset		 = 0;

	/*Configuración general del canal 6*/
	adc_ch6.Channel		 = ADC_CHANNEL_6;
	adc_ch6.Rank		 = 2;							//Segunda conversión a hacer
	adc_ch6.SamplingTime = ADC_SAMPLETIME_84CYCLES;		//Cantidad de ciclos a pasar para hacer el muestreo (carga del capacitor)
	adc_ch6.Offset		 = 0;

	/*Cargar las configuraciones en los registros FSR del MCU */
	HAL_ADC_ConfigChannel(&hadc1, &adc_ch5);
	HAL_ADC_ConfigChannel(&hadc1, &adc_ch6);

	/*Registrar la interrupción en el NVIC*/
	HAL_NVIC_EnableIRQ(ADC_IRQn);

}


/*
 * adc_process
 * Compara las lecturas actuales del ADC con las anteriores.
 * Filtra el ruido eléctrico y solicita actualizar la pantalla si hay un cambio real.
 */
static uint8_t adc_process(void){

	int diff_x = abs(raw_adc_x - actual_x);
	int diff_y = abs(raw_adc_y - actual_y);

	// Usamos if-else para evaluar el umbral y retornar el resultado
	if(diff_x > 40 || diff_y > 40){

		actual_x = raw_adc_x;
		actual_y = raw_adc_y;

		return 1; // Sí hubo cambio

	}

	else{

		return 0; // No hubo cambio

	}
}


/*
 * i2c1_display_Init
 * Configura los pines PB8 (SCL) y PB9 (SDA) para el uso de I2C del display
 */
static void i2c1_display_Init(void){

	/*Configuración de pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_Init_display = {0};

	/*Habilitar reloj de GPIOB en el bus AHB1*/
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configuración general de los pines: PB8 y PB9*/
	GPIO_Init_display.Pin		= GPIO_PIN_8 |
								  GPIO_PIN_9;
	GPIO_Init_display.Mode 	 	= GPIO_MODE_AF_OD;					//Establece los pines en modo alternativo Open-Drain (Requerido para I2C)
	GPIO_Init_display.Pull	 	= GPIO_PULLUP;						//Activa resisetncia Pull-Up interna para evitar estados flotantes
	GPIO_Init_display.Speed	 	= GPIO_SPEED_FREQ_VERY_HIGH;		//Configuración de velocidad muy alta para flancos de subida y bajada limpios
	GPIO_Init_display.Alternate = GPIO_AF4_I2C1;					//Función alternativa correspondiente a AF4 en I2C1

    /*Cargar la configuracion en los registros FSR del MCU*/
    HAL_GPIO_Init(GPIOB, &GPIO_Init_display);

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

/*
 * LCD_SendNibble
 *
 */
static void LCD_SendNibble(uint8_t nibble, uint8_t rs){

	/*Limpieza de los bytes control: RS, RW, E y BL*/
	uint8_t dato = (nibble & 0xF0) | LCD_BACKLIGHT | rs;		//0xF0 (11110000)

	/*Envío con Enable en 0 (Deja el byte listo en el bus, en reposo)*/
	HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &dato, 1, 10);		//10 ms como limite de espera

	/*Cambio del Enable a 1 (Pasa a nivel alto, el HD44780 empieza a "mirar" el dato)*/
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
 * Después de los 3 envíos de 0x30 se lleva al modo 8 bits (Esperando el primer nibble de una nueva instrucción)
 */
static void LCD_Init(void){

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
	LCD_SendCommand(0x08);		//0x28 = 00001000

	/*Display clear: Borra toda la DDRAM y regresa el cursor a la posicion 0*/
	LCD_SendCommand(0x01);		//0x01 = 00000001
	HAL_Delay(2);				//Esta es una operacion interna lenta, requiere espera adicional

	/*Entry mode set: Incrementa el cursor automaticamente (I/D = 1) y sin desplazamiento de pantalla (S = 0)*/
	LCD_SendCommand(0x06);		//0x06 = 00000110

	/*Display on: Display encendido (D = 1), cursor apagado (C = 0) y blink apagado (B = 0)*/
	LCD_SendCommand(0x0C);		//0x0C = 00001100

}


/*
 * LCD_SetCursor
 * Posiciona el cursor en una fila (0-3) y columna (0-19) especifica
 * Usa las direcciones base de cada linea segun el mapa DDRAM del datasheet
 */
static void LCD_SetCursor(uint8_t fila, uint8_t columna){

	uint8_t direccion;		//Dirección = address línea + # de columna

	switch(fila){

		case 0:

			direccion = LCD_LINE1 + columna;

			break;

		case 1:

			direccion = LCD_LINE2 + columna;

			break;

		case 2:

			direccion = LCD_LINE3 + columna;

			break;

		case 3:

			direccion = LCD_LINE4 + columna;

			break;

		default:

			direccion = LCD_LINE1 + columna;

			break;		//Va a la línea 1 en caso de llamar a una fila diferente de 0 a 3

	}

	/*Envío de al dirección con el comando Set DDRAM address*/
	LCD_SendCommand(0x80 | direccion);		//0x80 = 10000000

}


/*
 * LCD_Print
 * Escribe una cadena de texto completa a partir de la posicion actual del cursor,
 * Envía cada caracter con LCD_SendData()
 */
static void LCD_Print(char *texto){

	while(*texto != '\0'){

		LCD_SendData((uint8_t)(*texto));

		texto++;

	}

}


/*
 * LCD_Refresh
 * Actualiza unicamente el LCD con los 4 buffers de linea, sin transmitir por UART
 */
static void LCD_Refresh(void){

	LCD_SetCursor(0, 0);
	LCD_Print(lcd_linea0);

	LCD_SetCursor(1, 0);
	LCD_Print(lcd_linea1);

	LCD_SetCursor(2, 0);
	LCD_Print(lcd_linea2);

	LCD_SetCursor(3, 0);
	LCD_Print(lcd_linea3);

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
	HAL_UART_Receive_IT(&huart2, (uint8_t *) &rx_data, 1);		//El dato se guarda en la variable rx_data de a byte

}


/*
 * command_process
 * Evalúa el caracter recibido, ejecuta la acción correspondiente
 */
static void command_process(void){

    switch(rx_data){

        case 'H':

            HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);

            mco_actual = "HSI";

            break;

        case 'L':

            HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);

            mco_actual = "LSE";

            break;

        case 'P':

            HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);

            mco_actual = "PLL";

            break;

        case 'F':

            rtc_formato ^= 1; 	// Alterna entre 0 y 1

            break;

        case 'J':

            joystick_formato ^= 1; // Alterna entre 0 y 1

            break;

    }

}


/*
 * rtc_Init
 * Configura el RTC interno usando el LSE (32.768kHz) como fuente de reloj
 * Prescalers calculados para llegar a 1Hz: 32768 / 128 / 256 = 1Hz
 */
static void rtc_Init(void){

	/*Seleccionar LSE como fuente de reloj del RTC*/
	__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);

	/*Habilitar reloj de RTC en el bus APB1*/
	__HAL_RCC_RTC_ENABLE();

	/*Configuracion general del RTC*/
	hrtc.Instance            = RTC;
	hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv   = 128 - 1;							//Divisor asincrono: 32768 Hz / 128 = 256 Hz
	hrtc.Init.SynchPrediv    = 256 - 1;							//Divisor sincrono: 256 Hz / 256 = 1 Hz
	hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

	/*Inicializacion*/
	HAL_RTC_Init(&hrtc);

}


/*
 * rtc_Set
 * Configura hora y fecha solamente la primera vez que arranca el RTC
 * Se usa un registro de backup alimentado por la bateria conectada a VBAT
 * Debe tenerse en cuenta la "contraseña" para la inicialización del RTC
 */
static void rtc_Initial_Setting(void){

	/*Inicialización de estructuras*/
	RTC_TimeTypeDef RTC_Time = {0};
	RTC_DateTypeDef RTC_Date = {0};

	/*Verificar si el RTC ya fue inicializado antes*/
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_BACKUP_PSW){		//Si la contraseña es la misma sigue contando normalmente, de lo contrario se inicializa nuevamente

		/*Configuración inicial de la hora, se usa como referencia*/
		RTC_Time.Hours   = 19;
		RTC_Time.Minutes = 0;
		RTC_Time.Seconds = 0;

		/*Cargando la configuración en los registros FSR del MCU*/
		HAL_RTC_SetTime(&hrtc, &RTC_Time, RTC_FORMAT_BIN);

		/*Configuración inicial de la fecha, se usa como referencia*/
		RTC_Date.WeekDay = RTC_WEEKDAY_MONDAY;
		RTC_Date.Month   = RTC_MONTH_JULY;
		RTC_Date.Date    = 27;
		RTC_Date.Year    = 26;

		/*Cargando la configuración en los registros FSR del MCU*/
		HAL_RTC_SetDate(&hrtc, &RTC_Date, RTC_FORMAT_BIN);

		/*Se guarda en el registro de backup la contraseña*/
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_BACKUP_PSW);

	}
}


/*
 * rtc_process
 * Retorna 1 si cambió el segundo, 0 si no.
 */
static uint8_t rtc_process(void){

    HAL_RTC_GetTime(&hrtc, &hora_actual, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &fecha_actual, RTC_FORMAT_BIN);

    if(hora_actual.Seconds != rtc_ultimo_segundo){

        rtc_ultimo_segundo = hora_actual.Seconds;

        sprintf(lcd_linea0, "Fecha:%02d/%02d/%02d", fecha_actual.Date, fecha_actual.Month, fecha_actual.Year);

        if(rtc_formato == 0){

            sprintf(lcd_linea1, "Hora:%02d:%02d:%02d   ", hora_actual.Hours, hora_actual.Minutes, hora_actual.Seconds);

        }

        else{

            uint8_t h_12 = hora_actual.Hours % 12;
            char* am_pm;

            if (hora_actual.Hours >= 12) {

                am_pm = "PM"; // Es pasado el mediodía (PM)

                if (hora_actual.Hours == 12) {

                    h_12 = 12; // El mediodía se queda en 12

                }

                else {

                    h_12 = hora_actual.Hours - 12; // A las 13:00, 14:00, etc., le restamos 12

                }

            }

            else {

                am_pm = "AM"; // Es antes del mediodía (AM)

                if (hora_actual.Hours == 0) {

                    h_12 = 12; // La medianoche (00:00) se muestra como 12

                }

                else {

                    h_12 = hora_actual.Hours; // De la 1:00 a las 11:00 se queda igual

                }

            }


            sprintf(lcd_linea1, "Hora:%02d:%02d:%02d %s", h_12, hora_actual.Minutes, hora_actual.Seconds, am_pm);

        }

        return 1; // El segundo cambió

    }

    else {

        return 0; // Sigue en el mismo segundo

    }

}


/*
 * update_data
 * Toma los 4 buffers de linea armados en el main, actualiza el display y el mensaje de comincación serial
 */
static void update_data(void){

	char valor_x[16];
	char valor_y[16];

	/* 1. Formatear los datos del Joystick para UART y LCD simultáneamente */
	if (joystick_formato == 0) {

		float v_x = ((3.3f / 4095.0f) * (float) raw_adc_x);
		float v_y = ((3.3f / 4095.0f) * (float) raw_adc_y);

		sprintf(valor_x,"%.1f V", v_x);
		sprintf(valor_y,"%.1f V", v_y);

		// Actualizamos también el buffer de la LCD para la línea 2
		sprintf(lcd_linea2, "X:%3.1f V Y:%3.1f V", v_x, v_y);

	}

	else {

		float norm_x = ((float) raw_adc_x - 2048.0f) / 2048.0f;
		float norm_y = ((float) raw_adc_y - 2048.0f) / 2048.0f;

		sprintf(valor_x,"%.2f", norm_x);
		sprintf(valor_y,"%.2f", norm_y);

		// Actualizamos el buffer de la LCD
		sprintf(lcd_linea2, "X:%5.2f Y:%5.2f", norm_x, norm_y);

	}

	// La línea 3 (MCO1) se puede actualizar aquí directamente
	sprintf(lcd_linea3, "MCO1:%s", mco_actual);

	/* 2. Actualizar físicamente la pantalla LCD */
	LCD_Refresh();

	/* 3. Armar y enviar el mensaje UART */
	sprintf((char*) msg_buffer,
	"---------------------\r\n"
	" Datos actuales\r\n"
	"  X = %s\r\n"
	"  Y = %s\r\n"
	"  MCO1 = %s\r\n"
	"----------------------\r\n"
	"\r\n", valor_x, valor_y, mco_actual);

	HAL_UART_Transmit(&huart2, msg_buffer, strlen((char*) msg_buffer), 100);

}


/*
 * mco1_Init
 * Configura el MCO1 como salida para leer el HSI, LSE o PLL (Según se requiera) en PA8
 */
static void mco1_Init(void){

	/*Configuración de los pines*/
	/*Inicialización de estructuras*/
	GPIO_InitTypeDef GPIO_mco1_Init = {0};

	/*Habilitar reloj de GPIOA en el bus AHB1*/
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configuración general de los pines: PA8*/
	GPIO_mco1_Init.Pin	     = GPIO_PIN_8;
	GPIO_mco1_Init.Mode 	 = GPIO_MODE_AF_PP;				//Establece los pines en modo función alternativa
	GPIO_mco1_Init.Pull  	 = GPIO_NOPULL;					//Desactiva resistencias de Pull-Up o Pull-Down
	GPIO_mco1_Init.Speed	 = GPIO_SPEED_FREQ_VERY_HIGH;	//Configuración de velocidad como muy alta para evitar distorsiones
	GPIO_mco1_Init.Alternate = GPIO_AF0_MCO;					//Función alternativa correspondiente a AF0 en MCO1

	/*Cargar la configuracion en los registros FSR del MCU*/
	HAL_GPIO_Init(GPIOA, &GPIO_mco1_Init);

	/*Inicialización del MCO1*/
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);		//Se redirige la señal del Oscilador Interno de Alta Velocidad (HSI) a PA8

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
 * HAL_ADC_ConvCpltCallback
 * Callback de la conversión ADC
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){

	if(hadc->Instance == ADC1){

		if(adc_secuence == 0){

			raw_adc_x = HAL_ADC_GetValue(hadc);

			adc_secuence = 1;

		}

		else{

			raw_adc_y = HAL_ADC_GetValue(hadc);

			adc_secuence = 0;

			adc_done = 1;

		}

	}

}


/*
 * HAL_UART_RxCpltCallback
 * Callback de la recepción USART
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

    if (huart->Instance == USART2){

        usart_done = 1;

    }

}




