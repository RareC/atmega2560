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
	DDRG |= 0x20;
    init_pwm();
	init_adc();
    while (1) {
		change_speed();
		_delay_ms(20);
    } 
}

void init_pwm(){
	TCCR0A |= (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);	//Set PWM in non-inverting mode, and to re-set pin at top of counter
	TCCR0B |= (1<<CS01) | (1<<CS00);				//Use f/64 pre-scaler for frequency - arbitrary
}

void init_adc()
{
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//enable ADC, set ADC freq pre-scaler = F_CPU/128
	ADMUX |= (1<<REFS0)|(1<<ADLAR);					//AVcc ref & left adjust result, using ADC0
}

void change_speed(){
	ADCSRA |= (1<<ADSC);							//begin conversion
	while (ADCSRA & (1<<ADSC));						//wait for conversion to finish
	
	if (ADCH > 0x8A){								//Forwards
		TCCR0A &= ~(1<<COM0B1);						//disable PWM output on OCR0B if not already
		TCCR0A |= (1<<COM0A1);						//enable PWM output on OCR0A
		OCR0A = ADCH;								
	}		
	
	else if (ADCH <= 0x8A && ADCH >= 0x7D){			//Standstill
		TCCR0A &= ~((1<<COM0A1)|(1<<COM0B1));		//Disable both PWM outputs
	}
	
	else {											//Backwards
		TCCR0A &= ~(1<<COM0A1);						//Disable PWM output on OCR0A
		TCCR0A |= (1<<COM0B1);						//enable PWM output on ORC0B
		OCR0B = 0xFF - ADCH;						//ADCH is 8 bit value & low values mean higher reverse speed
	}
}
