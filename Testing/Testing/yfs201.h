/*
 * yfs201.h
 *
 *  Author: Blas
 */

#ifndef yfs201_h
#define yfs201_h

#include <avr/io.h>
#include <avr/interrupt.h>

void yfs201_timer0_init(void); //opcional, para poder medir caudal 0
void yfs201_timer1_init(void); //podria ser static interno al driver
void yfs201_init(void);
float yfs201_getFlow();

#endif 
