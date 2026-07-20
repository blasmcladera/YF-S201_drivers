/*
 * yfs201.c
 *
 *  Author: Blas
 */ 

#include "yfs201.h"

// G = Fclk / (Prescaler * 7.5 Hz)
const float G = 100000.0f / 3.0f;
static volatile uint16_t t1=0;
static volatile uint16_t t2=0; 
static volatile uint16_t delta = 0;
static volatile uint8_t cont_ms = 0;
static volatile uint8_t count = 0;
static volatile uint8_t skip = 0;

void yfs201_timer0_init(void){
	//Timer 0 CTC mode
	TCCR0A &= ~((1 << WGM00));
	TCCR0A |= (1 << WGM01);
	TCCR0B &= ~((1 << WGM02));
	
	//Prescaler 64
	TCCR0B &= ~(1 << CS12);
	TCCR0B |= (1 << CS11) | (1 << CS10);
	
	//249 ticks = 1ms interruption
	OCR0A = 249;
	
	//Output Compare A interrupt enable
	TIMSK0 |= (1 << OCIE0A);
}

ISR (TIMER0_COMPA_vect){
	if (count) {
		if (++cont_ms > 150) {
			delta = 0;
			count = 0;
			skip = 1;
			t2=0;
		}
	}
}

void yfs201_timer1_init(void){
	//Timer 1 normal mode
	TCCR1A &= ~((1 << WGM11) | (1 << WGM10));
	TCCR1B &= ~((1 << WGM13) | (1 << WGM12));
	
	//Prescaler 64
	TCCR1B &= ~(1 << CS12);
	TCCR1B |= (1 << CS11) | (1 << CS10);
	
	//Rising edge Input Capture
	TCCR1B |= (1 << ICES1);
	
	//Noise filter
	TCCR1B |= (1 << ICNC1);
	
	//Input Capture interruption enable
	TIMSK1 |= (1 << ICIE1);	
}

ISR (TIMER1_CAPT_vect){
	//Period of the YF-S201 output signal;
	t1 = t2;
	t2 = ICR1;
	cont_ms = 0;
	count = 1;
	if (skip) {
		skip = 0;
		return;
	}
	delta = t2-t1;
}

void yfs201_init(void) {
	
	//PB0 Input Pull-Up
	DDRB &= ~(1 << PORTB0);
	PORTB |= (1 << PORTB0);
	
	//Timers configuration
	yfs201_timer0_init();
	yfs201_timer1_init();
}

float yfs201_getFlow(){
	cli();
	uint16_t d = delta;
	sei();
	return d ? G / d : 0.0f;
}