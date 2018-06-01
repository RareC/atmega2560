/*###################################################################################################################
Created: 07/05/2018
Author: Chris
Description: Demo of using USART peripheral of MEGA2560
####################################################################################################################*/ 

#define F_CPU 16000000
#define BUFF_LEN 100

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void UART_INIT();
uint8_t UART_RX();
void UART_TX(uint8_t);
void UART_TX_STR(uint8_t*);

volatile uint8_t rx_buff[BUFF_LEN] = "default";		//volatile for use in ISR, preloaded with string so something is sent by default
volatile uint8_t rx_counter = 0;					//counter for how many bytes have been received

ISR(USART0_RX_vect){
	 rx_buff[rx_counter]=UDR0;
	 rx_counter++;
	 for (uint8_t i = rx_counter; i < BUFF_LEN; i++){
		 rx_buff[i] = '\0';
	 }
}

int main(void){
    UART_INIT();
	
    while (1) {
		UART_TX_STR(rx_buff);
		sei();										//enable interrupts during delay so new data can be passed
		_delay_ms(2000);							//data will be updated here
		cli();										//disable interrupts so data isn't modified during processing
		rx_counter = 0;								//needs to be reset each cycle
    }
}

//Initialize UART
void UART_INIT (){
	UBRR0H = 0x00;									//For 16 MHz clock & 9600 baud...
	UBRR0L = 0x67;									//UBBR = 103d = 0000 0110 0111
	UCSR0B|= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);		//Receive interrupt + RX/TX enable
	UCSR0C|= (1<<UCSZ01)|(1<<UCSZ00);				//8 bit data, 1 stop bit and no parity
}

void UART_TX (uint8_t data){
	while (!( UCSR0A & (1<<UDRE0)));			//wait until register is free
	UDR0 = data;								//load data in the register
	while (!(UCSR0A & (1<<TXC0)));				//wait until data has been transmitted
}

//send multiple characters
void UART_TX_STR (uint8_t *str){
	uint8_t i=0;
	while (*(str+i))
	{
		UART_TX(*(str+i));
		i++;
	}
}