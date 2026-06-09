/*
 * Tarea_2.c
 *
 *  Created on: Jun 6, 2026
 *      Author: Sophia Alejandra Velasquez Fuentes - sovelasquezf@unal.edu.co
 *       Brief: Contador con fotocompuertas
 */

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdio.h>

/*Variables*/
typedef enum {
	CERO,
	UNO,
	DOS,
	TRES,
	CUATRO,
	CINCO,
	SEIS,
	SIETE,
	OCHO,
	NUEVE
} numero;		//Tipo de variable numero con los numeros del 0 al 9

uint8_t unidades = 0;
uint8_t decenas = 0;
uint8_t centenas = 0;
uint8_t unidades_mil = 0;

uint8_t dig1 = 0;
uint8_t dig2 = 0;
uint8_t dig3 = 0;
uint8_t dig4 = 0;
volatile uint8_t digito = 0;

volatile uint8_t refresco_bandera = 0;

volatile uint16_t contador = 0;

volatile uint8_t incremento_bandera = 0;
volatile uint8_t decremento_bandera = 0;

/*Cabeceras de funciones*/
volatile void led_ok(void);
void init_gpio(void);
void dibujar_numero(numero);
void separar(uint16_t);
void init_timer3(void);
void refresco_digitos(void);
void init_exti(void);

int main(void){

	led_ok();
	init_gpio();
	init_timer3();
	init_exti();


	while(1){

		/*Separacion del contador actual*/
		separar(contador);


		/*Incremento del contador*/
		if(incremento_bandera){

			contador++;

			incremento_bandera = 0;

		}


		/*Decremento del contador*/
		if(decremento_bandera){

			contador--;

			decremento_bandera = 0;

		}


		/*Refresco de los digitos con el valor del contador*/
		if(refresco_bandera){

			refresco_digitos();

			refresco_bandera = 0;
		}


	}

	return 0;
}


/*Funciones*/

/*Led_ok titilando a 2 Hz (500ms)*/
volatile void led_ok(void){

	/*Configuracion del GPIO*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;			//Activando la señal de reloj para GPIOH

	GPIOH->MODER &= ~(GPIO_MODER_MODE1);			//Limpiando el registro
	GPIOH->MODER |= GPIO_MODER_MODE1_0;				//Configurando como salida de proposito general

	GPIOH->OTYPER &= ~(GPIO_OTYPER_OT1);			//Configurando como salida Push-Pull

	GPIOH->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED1);		//Limpiando el registro
	GPIOH->OSPEEDR |= GPIO_OSPEEDR_OSPEED1_1;		//Configurando como salida a velocidad alta

	GPIOH->PUPDR &= ~(GPIO_PUPDR_PUPD1);			//Configurando como no Pull-Up/Pull-Down

	GPIOH->ODR |= GPIO_ODR_OD1;						//Encendiendo el led en PH1

	/*Configuracion del TIM2*/
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		//Activando la señal de reloj para el TIM2

	TIM2->PSC = 16000 - 1;					//Configurando el Prescale a 10 Hz o 1 ms

	TIM2->ARR = 500 - 1;					//Configurando el auto-load a 500 ms o 0.5 seg

	TIM2->CNT = 0;							//Reiniciando el contador

	TIM2->SR &= ~(TIM_SR_UIF);				//Limpieza de la bandera de interrupcion


	TIM2->DIER &= ~(TIM_DIER_UIE);			//Limpieza del registro
	TIM2->DIER |= TIM_DIER_UIE;				//Configuracion de la interrupcion como Update event

	__NVIC_EnableIRQ(TIM2_IRQn);			//Matriculando la interrupcion del TIM2 en el NVIC

	TIM2->CR1 &= ~(TIM_CR1_DIR);			//Configurando la direccion del counter como upcounter

	TIM2->CR1 &= ~(TIM_CR1_ARPE);			//Limpiando el registro
	TIM2->CR1 |= TIM_CR1_ARPE;				//Activando la precarga del ARR

	TIM2->CR1 |= TIM_CR1_CEN;				//Activacion del contador

}


/*Funcion ISR para el TIM2 (led_ok)*/
void TIM2_IRQHandler(void){

	if(TIM2->SR & TIM_SR_UIF){			//Verifica que si se levante una bandera

		GPIOH->ODR ^= GPIO_ODR_OD1;		//Hace el toogle en el estado del led

		TIM2->SR &= ~(TIM_SR_UIF);		//Limpieza del bit (baja la bandera)

	}

}


/*Configuracion de puertos GPIO*/
void init_gpio(void){

	/*Salidas*/
	/*Puerto A: PA0, PA1, PA4, PA10*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;																						//Activando la señal de reloj para GPIOA

	GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE4 | GPIO_MODER_MODE10);								//Limpiando el registro
	GPIOA->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE4_0 | GPIO_MODER_MODE10_0);						//Configurando como salida de proposito general

	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT4 | GPIO_OTYPER_OT10);									//Configurando como salida Push-Pull

	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1 | GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED10);			//Limpiando el registro
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0_1 | GPIO_OSPEEDR_OSPEED1_1 | GPIO_OSPEEDR_OSPEED4_1 | GPIO_OSPEEDR_OSPEED10_1);		//Configurando como salida a velocidad alta

	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD10);								//Configurando como no Pull-Up/Pull-Down

	GPIOA->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD4 | GPIO_ODR_OD10);												//Comienza encendido


	/*Puerto B: PB0, PB10, PB13, PB14, PB15*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;																													//Activando la señal de reloj para GPIOB

	GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE10 | GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15);									//Limpiando el registro
	GPIOB->MODER |= (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE10_0 | GPIO_MODER_MODE13_0 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0);							//Configurando como salida de proposito general

	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT10 | GPIO_OTYPER_OT13 | GPIO_OTYPER_OT14 | GPIO_OTYPER_OT15);										//Configurando como salida Push-Pull

	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED10 | GPIO_OSPEEDR_OSPEED13 | GPIO_OSPEEDR_OSPEED14 | GPIO_OSPEEDR_OSPEED15);				//Limpiando el registro
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0_1 | GPIO_OSPEEDR_OSPEED10_1 | GPIO_OSPEEDR_OSPEED13_1 | GPIO_OSPEEDR_OSPEED14_1 | GPIO_OSPEEDR_OSPEED15_1);		//Configurando como salida a velocidad alta

	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD13 | GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD15);									//Configurando como no Pull-Up/Pull-Down

	GPIOB->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD10 | GPIO_ODR_OD13 | GPIO_ODR_OD14 | GPIO_ODR_OD15);															//Comienza encendido


	/*Puerto C: PC1, PC4*/

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;										//Activando la señal de reloj para GPIOC

	GPIOC->MODER &= ~(GPIO_MODER_MODE1 | GPIO_MODER_MODE4);						//Limpiando el registro
	GPIOC->MODER |= (GPIO_MODER_MODE1_0 | GPIO_MODER_MODE4_0);					//Configurando como salida de proposito general

	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT1 | GPIO_OTYPER_OT4);						//Configurando como salida Push-Pull

	GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED1 | GPIO_OSPEEDR_OSPEED4);			//Limpiando el registro
	GPIOC->OSPEEDR |= (GPIO_OSPEEDR_OSPEED1_1 | GPIO_OSPEEDR_OSPEED4_1);		//Configurando como salida a velocidad alta

	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD4);						//Configurando como no Pull-Up/Pull-Down

	GPIOC->ODR &= ~(GPIO_ODR_OD1 | GPIO_ODR_OD4);								//Comienza encendido


	/*Entradas*/
	/*PB2 y PC8*/

	GPIOB->MODER &= ~(GPIO_MODER_MODE2);		//Configurando PB2 como entrada

	GPIOC->MODER &= ~(GPIO_MODER_MODE8);		//Configurando PC8 como entrada

	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD2);		//Configurando como no Pull-Up/Pull-Down a PB2

	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD8);		//Configurando como no Pull-Up/Pull-Down a PC8

}


/*Apagado y encendido de leds segun el numero a representar*/
void dibujar_numero(numero x){				//Se usa logica inversa por el uso de transistores PNP al ser un 7 segmentos de anodo comun

	switch(x){

	case CERO:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD14);		//PB14: Led E (encendido)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR |= GPIO_ODR_OD1;			//PA1: Led G (apagado)

		break;

	case UNO:
		GPIOA->ODR |= GPIO_ODR_OD4;			//PA4: Led A (apagado)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR |= GPIO_ODR_OD13;		//PB13: Led D (apagado)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR |= GPIO_ODR_OD1;			//PC1: Led F (apagado)
		GPIOA->ODR |= GPIO_ODR_OD1;			//PA1: Led G (apagado)

		break;

	case DOS:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR |= GPIO_ODR_OD0;			//PB0: Led C (apagado)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD14);		//PB14: Led E (encendido)
		GPIOC->ODR |= GPIO_ODR_OD1;			//PC1: Led F (apagado)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case TRES:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR |= GPIO_ODR_OD1;			//PC1: Led F (apagado)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case CUATRO:
		GPIOA->ODR |= GPIO_ODR_OD4;			//PA4: Led A (apagado)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR |= GPIO_ODR_OD13;		//PB13: Led D (apagado)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case CINCO:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR |= GPIO_ODR_OD0;			//PA0: Led B (apagado)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case SEIS:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR |= GPIO_ODR_OD0;			//PA0: Led B (apagado)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD14);		//PB14: Led E (encendido)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case SIETE:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR |= GPIO_ODR_OD13;		//PB13: Led D (apagado)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR |= GPIO_ODR_OD1;			//PC1: Led F (apagado)
		GPIOA->ODR |= GPIO_ODR_OD1;			//PA1: Led G (apagado)

		break;

	case OCHO:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD14);		//PB14: Led E (encendido)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	case NUEVE:
		GPIOA->ODR &= ~(GPIO_ODR_OD4);		//PA4: Led A (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD0);		//PA0: Led B (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD0);		//PB0: Led C (encendido)
		GPIOB->ODR &= ~(GPIO_ODR_OD13);		//PB13: Led D (encendido)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR &= ~(GPIO_ODR_OD1);		//PC1: Led F (encendido)
		GPIOA->ODR &= ~(GPIO_ODR_OD1);		//PA1: Led G (encendido)

		break;

	default:								//En caso de recibir un numero que no este entre 0 y 9
		GPIOA->ODR |= GPIO_ODR_OD4;			//PA4: Led A (apagado)
		GPIOA->ODR |= GPIO_ODR_OD0;			//PA0: Led B (apagado)
		GPIOB->ODR |= GPIO_ODR_OD0;			//PB0: Led C (apagado)
		GPIOB->ODR |= GPIO_ODR_OD13;		//PB13: Led D (apagado)
		GPIOB->ODR |= GPIO_ODR_OD14;		//PB14: Led E (apagado)
		GPIOC->ODR |= GPIO_ODR_OD1;			//PC1: Led F (apagado)
		GPIOA->ODR |= GPIO_ODR_OD1;			//PA1: Led G (apagado)

		break;

	}

}


/*Separacion del contador en digitos separados*/
void separar(uint16_t x){

	unidades_mil = (x / 1000) % 10;

	centenas = (x % 1000) / 100;

	decenas = (x % 100) / 10;

	unidades = (x % 10);

}


/*Configuracion del TIM3 a 6 ms*/
void init_timer3(void){

	/*Configuracion del TIM3*/
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;		//Activando la señal de reloj para el TIM3

	TIM3->PSC = 16000 - 1;					//Configurando el Prescale a 10 Hz o 1 ms

	TIM3->ARR = 6 - 1;						//Configurando el auto-load a 6 ms

	TIM3->CNT = 0;							//Reiniciando el contador

	TIM3->SR &= ~(TIM_SR_UIF);				//Limpieza de la bandera de interrupcion

	TIM3->DIER &= ~(TIM_DIER_UIE);			//Limpieza del registro
	TIM3->DIER |= TIM_DIER_UIE;				//Configuracion de la interrupcion como Update event

	__NVIC_EnableIRQ(TIM3_IRQn);			//Matriculando la interrupcion del TIM3 en el NVIC

	TIM3->CR1 &= ~(TIM_CR1_DIR);			//Configurando la direccion del counter como upcounter

	TIM3->CR1 &= ~(TIM_CR1_ARPE);			//Limpiando el registro
	TIM3->CR1 |= TIM_CR1_ARPE;				//Activando la precarga del ARR

	TIM3->CR1 |= TIM_CR1_CEN;				//Activacion del contador

}


/*Funcion ISR para el TIM3 (tasa de refresco)*/
void TIM3_IRQHandler(void){

	if(TIM3->SR & TIM_SR_UIF){			//Verifica que si se levante una bandera

		refresco_bandera = 1;

		TIM3->SR &= ~(TIM_SR_UIF);		//Limpieza del bit (baja la bandera)

	}

}


/*Encendido y apagado de digitos segun la tasa de refresco y el numero en el contador*/
void refresco_digitos(void){

	GPIOA->ODR |= GPIO_ODR_OD10;		//PA10: Digito 1 (apagado)
	GPIOC->ODR |= GPIO_ODR_OD4;			//PC4: Digito 2 (apagado)
	GPIOB->ODR |= GPIO_ODR_OD10;		//PB10: Digito 3 (apagado)
	GPIOB->ODR |= GPIO_ODR_OD15;		//PB15: Digito 4 (apagado)

	switch(digito){

	case 0:

		dibujar_numero((numero)unidades_mil);

		GPIOA->ODR &= ~(GPIO_ODR_OD10);		//PA10: Digito 1 (encendido)
		GPIOC->ODR |= GPIO_ODR_OD4;			//PC4: Digito 2 (apagado)
		GPIOB->ODR |= GPIO_ODR_OD10;		//PB10: Digito 3 (apagado)
		GPIOB->ODR |= GPIO_ODR_OD15;		//PB15: Digito 4 (apagado)

		digito++;

		break;

	case 1:

		dibujar_numero((numero)centenas);

		GPIOA->ODR |= GPIO_ODR_OD10;		//PA10: Digito 1 (apagado)
		GPIOC->ODR &= ~(GPIO_ODR_OD4);		//PC4: Digito 2 (encendido)
		GPIOB->ODR |= GPIO_ODR_OD10;		//PB10: Digito 3 (apagado)
		GPIOB->ODR |= GPIO_ODR_OD15;		//PB15: Digito 4 (apagado)

		digito++;

		break;

	case 2:

		dibujar_numero((numero)decenas);

		GPIOA->ODR |= GPIO_ODR_OD10;		//PA10: Digito 1 (apagado)
		GPIOC->ODR |= GPIO_ODR_OD4;			//PC4: Digito 2 (apagado)
		GPIOB->ODR &= ~(GPIO_ODR_OD10);		//PB10: Digito 3 (encendido)
		GPIOB->ODR |= GPIO_ODR_OD15;		//PB15: Digito 4 (apagado)

		digito++;

		break;

	case 3:

		dibujar_numero((numero)unidades);

		GPIOA->ODR |= GPIO_ODR_OD10;		//PA10: Digito 1 (apagado)
		GPIOC->ODR |= GPIO_ODR_OD4;			//PC4: Digito 2 (apagado)
		GPIOB->ODR |= GPIO_ODR_OD10;		//PB10: Digito 3 (apagado)
		GPIOB->ODR &= ~(GPIO_ODR_OD15);		//PB15: Digito 4 (encendido)

		digito = 0;

		break;

	default:

		digito = 0;

		break;

	}

}


/*Configuracion de las EXTI2 y EXTI8*/
void init_exti(void){
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;				//Encendiendo señal de reloj para el SYSCFG

	/*EXTI2 para PB2*/
	SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI2);		//Limpiando el registro del MUX
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB;		//Configurando el EXTI para el puerto B (PB2)

	EXTI->RTSR |= EXTI_RTSR_TR2;						//Configurando para detectar flancos de subida (cuando entra la interrupcion)

	EXTI->IMR |= EXTI_IMR_IM2;							//Activacion de la interrupcion

	EXTI->PR |= EXTI_PR_PR2;							//Limpiando la bandera relacionada al EXTI2

	__NVIC_EnableIRQ(EXTI2_IRQn);						//Registro de la interrupcion en el NVIC


	/*EXTI8 para PC8*/
	SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8);		//Limpiando el registro del MUX
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PC;		//Configurando el EXTI para el puerto C (PC8)

	EXTI->FTSR |= EXTI_FTSR_TR8;						//Configurando para detectar flancos de bajada (cuando sale la interrupcion)

	EXTI->IMR |= EXTI_IMR_IM8;							//Activacion de la interrupcion

	EXTI->PR |= EXTI_PR_PR8;							//Limpiando la bandera relacionada al EXTI8

	__NVIC_EnableIRQ(EXTI9_5_IRQn);						//Registro de la interrupcion en el NVIC

}


/*Funcion ISR para el EXTI2 (incremento de contador)*/
void EXTI2_IRQHandler(void){
	if(EXTI->PR & EXTI_PR_PR2){

		EXTI->PR |= EXTI_PR_PR2;		//Limpiando la bandera relacionada al EXTI2

		incremento_bandera = 1;

	}

}


/*Funcion ISR para el EXTI8 (incremento de contador)*/
void EXTI9_5_IRQHandler(void){
	if(EXTI->PR & EXTI_PR_PR8){

		EXTI->PR |= EXTI_PR_PR8;		//Limpiando la bandera relacionada al EXTI8

		decremento_bandera = 1;

	}

}




