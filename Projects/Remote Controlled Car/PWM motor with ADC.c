/*
 * MotorPWM.c
 *
 * Created: 03/09/2018 14:17:07
 * Author : Chris
 */ 

#define F_CPU 16000000
#define ADATE 5

#include <avr/io.h>
#include <util/delay.h>


void init_pwm();
void init_adc();
void change_speed();

int main(void){
	DDRB |= 0x80;
    init_pwm();
	init_adc();
    while (1) {
		change_speed();
    }
}

void init_pwm(){
	TCCR0A |= (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);		//Set PWM in non-inverting mode, and to re-set pin at top of counter
	TCCR0B |= (1<<CS01) | (1<<CS00);					//Use f/64 pre-scaler for frequency - arbitrary
	OCR0A = 0xFF;										//Duty Cycle of 25%
}

void init_adc()
{
	ADCSRA |= (1<<ADEN);							//enable ADC
	ADMUX |= (1<<REFS0)|(1<<ADLAR);													//use 2.56V ref & left adjust result, using ADC0
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);    //set ADC freq pre-scaler = F_CPU/128
}

void change_speed(){
	ADCSRA |= (1<<ADSC);							//begin conversion
	while (ADCSRA & (1<<ADSC)){}					//wait for conversion
	OCR0A = ADCH;		
}