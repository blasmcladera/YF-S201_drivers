#include "uart.h"

void UART0_init(void){
	//función de inicialización de uart
	
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);	 //configura formato 8N1
	UBRR0L = 103;						 //configura BAUDRATE = 9600bps@16MHz
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);	// Activa el Transmisor del Puerto Serie
}

void UART0_sendString(const char* str){
	flag_uart_txlleno=1;
	if (UCSR0B & (1 << UDRIE0)) {
		return; // No se pudo mandar porque el buffer TX estaba lleno LE SAQUE EL 2!!!!!!
	}
	//función para guardar en un buffer el string y enviarlo de a poco
	strncpy(buffer_tx, str, BUFFER_SIZE - 1);
	// Nos aseguramos de que termine con el caracter nulo
	buffer_tx[BUFFER_SIZE - 1] = '\0';
	
	UCSR0B |= (1<<UDRIE0);
	
}

volatile uint8_t tx_index=0;

ISR(USART_UDRE_vect){
	//interrupción de transmisión (el registro esta vacio)
	if(buffer_tx[tx_index]!='\0'){
		UDR0 = buffer_tx[tx_index];
		tx_index++;
	}else{
		tx_index=0;
		UCSR0B &= ~(1 << UDRIE0); // cuando termina de transmitir, enmascara la interrupción y así evita el bucle infinito
		flag_uart_txlleno=0;
	}
}