/**************************
Universidad del Valle de Guatemala
Electrónica digital 2
Proyecto: Lab 3
Hardware: ATMEGA328p
Created: 19/07/2024 10:52:46
Author : James Ramírez
***************************/

#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include "ADC/ADC.h"
#include "SPI_SLAVE/SPI_SLAVE.h"
#include <avr/interrupt.h>

// Variables globales para almacenar los resultados del ADC
volatile uint8_t current_adc_channel = 1;  // Canal ADC actual
volatile uint8_t adc_result1, adc_result2; // Valores ADC para los dos canales

// Interrupción del ADC, se ejecuta cuando una conversión ADC se completa
ISR(ADC_vect) {
	if (current_adc_channel == 1) {
		adc_result1 = ADC;  // Leer valor del ADC en el canal 1
		current_adc_channel = 2;  // Cambiar al canal 2
		selectADCChannel2();  // Configurar el ADC para leer del canal 2
		} else {
		adc_result2 = ADC;  // Leer valor del ADC en el canal 2
		current_adc_channel = 1;  // Cambiar al canal 1
		selectADCChannel1();  // Configurar el ADC para leer del canal 1
	}
	startConversion();  // Iniciar una nueva conversión ADC
}

int main(void) {
	// Inicializar la SPI como esclavo
	SpiInit(SPI_SLAVE_SS, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOK_FIRST_EDGE);

	// Configurar el ADC con un prescaler de 128
	ADC_init(128);
	
	// Iniciar la primera conversión ADC en el canal 1
	startConversion();
	selectADCChannel1();

	// Habilitar interrupciones SPI
	SPCR |= (1 << SPIE);

	// Habilitar interrupciones globales
	sei();

	while (1) {
		// Leer el valor recibido por SPI
		uint8_t receivedSpiData = SPDR;
		
		// Responder con el valor ADC apropiado basado en el comando recibido
		if (receivedSpiData == 'c') {
			SpiWrite(adc_result1);  // Enviar valor ADC del canal 1
			_delay_ms(1);  // Pequeño retardo para estabilizar la comunicación
			} else if (receivedSpiData == 'd') {
			SpiWrite(adc_result2);  // Enviar valor ADC del canal 2
			_delay_ms(1);  // Pequeño retardo para estabilizar la comunicación
		}
	}
}
