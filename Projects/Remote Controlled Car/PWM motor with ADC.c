/*
 * MotorPWM.c
 *
 * Created: 03/09/2018 14:17:07
 * Author : Chris
 */ 

#define F_CPU 16000000
#define ADATE 5
#define VHIGH 0x8A
#define VLOW 0x6D

#include <avr/io.h>
#include <util/delay.h>

void init_pwm();
void init_adc();
void change_speed();

void UART_INIT();
void UART_TX(uint8_t);
void UART_TX_STR(uint8_t *str);

int main(void){
	DDRB |= 0xE0;
	DDRG |= 0x20;
    init_pwm();
	init_adc();
	UART_INIT();
    while (1) {
		change_speed();
		_delay_ms(10);
    } 
}

void init_pwm(){
	TCCR0A |= (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);	//Set timer0 in PWM non-inverting mode, and to re-set pin at top of counter
	TCCR0B |= (1<<CS01)|(1<<CS00);					//Use f/64 pre-scaler for frequency - arbitrary
	
	TCCR1A |= (1<<COM1A1)|(1<<WGM10);				//Set up timer1 the same as timer0 - timer1 is 16 bit!! WGM bits set differently so to use 8bit mode
	TCCR1B |= (1<<WGM12)|(1<<CS11)|(1<<CS10);
}

void init_adc()
{
	ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//enable ADC, set ADC freq pre-scaler = F_CPU/128
	ADMUX |= (1<<REFS0)|(1<<ADLAR);							//AVcc as ref & left adjust result
}

void change_speed(){
	ADMUX &= ~(1<<MUX0);							//Use ADC0
	ADCSRA |= (1<<ADSC);							//begin conversion
	while (ADCSRA & (1<<ADSC));						//wait for conversion to finish
	uint8_t Vx = ADCH;
	
	ADMUX |= (1<<MUX0);								//Use ADC1
	ADCSRA |= (1<<ADSC);							//begin conversion
	while (ADCSRA & (1<<ADSC));
	uint8_t Vy = ADCH;
	
	//Alter power between left and right wheels according to values of Vx and Vy
	//Needs some control added to ensure wheels are spinning at correct speed - one wheel cuts out way sooner than the other
	
	if (Vy > VHIGH){								//Forwards
		TCCR0A &= ~(1<<COM0B1);						//disable PWM output on OCR0B
		TCCR0A |= (1<<COM0A1);						//enable PWM output on OCR0A
		TCCR1A &= ~(1<<COM1B1);						//disable PWM output on OCR1B
		TCCR1A |= (1<<COM1A1);						//enable PWM output on OCR1A
		
		if (Vx >= 0x80){											//Turn right
			float pwr_R = 1;										//Full power to right wheel
			float pwr_L = 1 - (float)(Vx - 0x7F)/(0xFF - 0x7F);		//Less power to left wheel depending how far over the stick is
			OCR0A = Vy * pwr_R;
			OCR1A = Vy * pwr_L;
		}
		else{														//Turn left
			float pwr_L = 1;										//Full power to left wheel
			float pwr_R = 1- (float)(0xFF - Vx - 0x7F)/(0xFF - 0x7F);//Less power to right wheel depending on how far over stick is
			OCR0A = Vy * pwr_R;
			OCR1A = Vy * pwr_L;
		}
	}
	else if (Vy > VLOW){							//Backwards
		TCCR0A &= ~(1<<COM0A1);						//Disable OCR0A
		TCCR0A |= (1<<COM0B1);						//Enable OCR0B
		TCCR1A &= ~(1<<COM1A1);						//Disable OCR1A
		TCCR1A |= (1<<COM1B1);						//Enable OCR1B
		
		if (Vx >= 0x80){											//Turn right
			float pwr_L = 1;										//Full power to left wheel
			float pwr_R = 1- (float)(0xFF - Vx - 0x7F)/(0xFF - 0x7F);//Less power to right wheel depending on how far over stick is
			OCR0B = Vy * pwr_R;
			OCR1B = Vy * pwr_L;
		}
		else{														//Turn Left
			float pwr_R = 1;										//Full power to right wheel
			float pwr_L = 1 - (float)(Vx - 0x7F)/(0xFF - 0x7F);		//Less power to left wheel depending how far over the stick is
			OCR0B = Vy * pwr_R;
			OCR1B = Vy * pwr_L;			
		}
	/* Debug code for printing reg values
		uint_8 value [3]={};
		itoa(OCR0A,value,10);
		UART_TX_STR(value);
		UART_TX('\n');
	*/
	}
	else{											//Standstill
		TCCR0A &= ~(1<<COM0B1);						//disable PWM output on OCR0B
		TCCR0A &= ~(1<<COM0A1);						//disable PWM output on OCR0A
		TCCR1A &= ~(1<<COM1B1);						//disable PWM output on OCR1B
		TCCR1A &= ~(1<<COM1A1);						//disable PWM output on OCR1A
	}
	/*
	else {											//No forwards movement
		
		TCCR0A &= ~(1<<COM0A1);						//Disable PWM output on OCR0A
		TCCR0A |= (1<<COM0B1);						//enable PWM output on ORC0B
		OCR0B = 0xFF - ADCH;						//ADCH is 8 bit value & low values mean higher reverse speed
	} */
}


//Initialize UART - Below functions are for debug
void UART_INIT (){
	UBRR0H = 0x00;									//For 16 MHz clock & 9600 baud...
	UBRR0L = 0x67;									//UBBR(H+L) = 103d = 0000 0110 0111
	UCSR0B|= (1<<TXEN0);							//USART TX enable
	UCSR0C|= (1<<UCSZ01)|(1<<UCSZ00);				//8 bit data, 1 stop bit and no parity
}

void UART_TX (uint8_t data){
	while (!( UCSR0A & (1<<UDRE0)));			//wait until register is free
	UDR0 = data;								//load data in the register
	while (!(UCSR0A & (1<<TXC0)));				//wait until data has been transmitted
}

void UART_TX_STR (uint8_t *str){
	uint8_t i=0;
	while (*(str+i))
	{
		UART_TX(*(str+i));
		i++;
	}
}