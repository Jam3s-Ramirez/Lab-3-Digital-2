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
#include <stdio.h>
#include <avr/interrupt.h>
#include "SPI-Maestro/SPI-Maestro.h"
#include "USART/USART.h"

volatile uint8_t adcValue1, adcValue2;
volatile char serialBuffer[16];
int adcValue1_int, adcValue2_int;
uint8_t startup = 1;
int numericInput = 0;
char serialInput[4];

void initializeSystem() {
	// Configurar puertos como salida
	DDRC |= (1 << DDC4) | (1 << DDC5);
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	
	// Apagar LEDs inicialmente
	PORTC &= ~((1 << PORTC4) | (1 << PORTC5));
	PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7));
	
	// Inicializar SPI como maestro y UART
	SpiInit(SPI_MASTER_OSC_DIV128, SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOK_FIRST_EDGE);
	UART_init(9600);
}

void transmitADCValues() {
	adcValue2_int = (int)(adcValue2);
	adcValue1_int = (int)(adcValue1);
	UART_send_string("\n\r");
	snprintf(serialBuffer, sizeof(serialBuffer), "P1: %d \r\n", adcValue1_int);
	UART_send_string(serialBuffer);
	_delay_ms(1);
	snprintf(serialBuffer, sizeof(serialBuffer), "P2: %d \r\n", adcValue2_int);
	UART_send_string(serialBuffer);
}

void readADCValues() {
	PORTB &= ~(1 << PORTB2);
	SpiWrite('c');
	adcValue1 = SpiRead();
	_delay_ms(10);
	SpiWrite('d');
	adcValue2 = SpiRead();
	_delay_ms(10);
	PORTB |= (1 << PORTB2);
}

void handleSerialInput() {
	if (UART_available()) {
		char command = UART_receive();
		switch (command) {
			case 'm':
			transmitADCValues();
			_delay_ms(100);
			UART_send_string("\nEnter 'm' to see ADC values.\r\n");
			UART_send_string("Enter 'n' for the counter.\r\n");
			handleSerialInput();
			break;
			case 'n':
			handleCounterInput();
			_delay_ms(100);
			UART_send_string("\nEnter 'm' to see ADC values.\r\n");
			UART_send_string("Enter 'n' for the counter.\r\n");
			handleSerialInput();
			break;
			default:
			UART_send_string("\nInvalid command received.\n");
			break;
		}
	}
}

void updatePortValues(uint8_t value) {
	PORTC = (PORTC & ~((1 << PORTC4) | (1 << PORTC5))) | ((value & 0b00000011) << PORTC4);
	PORTD = (PORTD & ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7))) | ((value & 0b11111100) << (PORTD2 - 2));
}

void handleCounterInput() {
	UART_send_string("\n\rPlease enter a number (0-255): ");
	UART_receive_string(serialInput, sizeof(serialInput));
	UART_send_string("\nReceived string: ");
	UART_send_string(serialInput);
	numericInput = atoi(serialInput);
	UART_send_string("\nReceived value as integer: ");
	UART_send_number(numericInput);
	if (numericInput >= 0 && numericInput <= 255) {
		updatePortValues(numericInput);
		} else {
		UART_send_string("\nInvalid value. Please enter a number between 0 and 255.\n");
	}
}

int main(void) {
	initializeSystem();
	sei();  // Habilitar interrupciones globales
	
	while (1) {
		if (startup) {
			_delay_ms(100);
			UART_send_string("\nEnter 'm' to see ADC values.\r\n");
			UART_send_string("Enter 'n' for the counter.\r\n");
			readADCValues();
			_delay_ms(300);
			handleSerialInput();
			startup = 0;
			} else {
			_delay_ms(500);
			handleSerialInput();
		}
		readADCValues();
		_delay_ms(300);
	}
}
