/*
 * IR Tx.c
 *
 * Created: 31/08/2018 16:55:36
 * Author : Chris
 */ 

#include <avr/io.h>

void init_timer();

int main(void)
{
    init_timer();
    while (1) 
    {
    }
}

void init_timer(){
	DDRB |= 0xFF;					  //sets PB7 as output 
	TCCR0A |= (1<<COM0A0)|(1<<WGM01); //Toggles pin OC0A on compare match (dig. 13/PB7) & sets timer 0 to clear on compare match to OCRA
	TCCR0B |= (1<<CS00);			  //Sets timer0 to use full 16MHz clock
	OCR0A = 0xD1;					  //Sets output compare register to 209 - meaning 210 clock cycles are counted between compare matches, which is ~13us
	
}

