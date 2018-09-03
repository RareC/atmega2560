/*###################################################################################################################
Created: 01/02/18
Author: Chris
Description: Demo code of LED matrix 
Hardware Pin Connections: PB1 = SCK, PB2 = MOSI, PB4 = CS, PF0 = analogue input
- ADC used to set brightness of LED's 
####################################################################################################################*/ 

#define F_CPU 16000000

#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <avr/io.h>
#include <util/delay.h>
#include <LEDMatrix.h>



void init_adc()
{
	ADMUX |= (1<<REFS0)|(1<<ADLAR);										//use internal 2.56V ref & left adjust result, use ADC0
	ADCSRA |= (1<<ADEN)|(1 << ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADSC);   //enable ADC, put into free running mode & set ADC freq pre-scaler = F_CPU/128. Begin conversions

}


void print_HI(uint8_t sc)
{
	uint8_t H[7]={0x00,0x00,0x7E,0x08,0x08,0x7E,0x00};						//[0] is rightmost column and [7] leftmost, MSB is top row and LSB is bottom row
	uint8_t I[7]={0x00,0x00,0x00,0x00,0x5E,0x00,0x00};						//depends which way around you look at it of course!

	flash(H,sc);
	_delay_ms(1000);

	flash(I,sc);
	_delay_ms(1000);
}

void print_BRITT(uint8_t sc)
{
	uint8_t B[7]={0x00,0x00,0x2C,0x52,0x52,0x7E,0x00};
	uint8_t R[7]={0x00,0x00,0x20,0x20,0x3E,0x00,0x00};
	uint8_t I[7]={0x00,0x00,0x00,0x00,0x5E,0x00,0x00};
	uint8_t T[7]={0x00,0x00,0x00,0x12,0x7E,0x10,0x00};

	flash(B,sc);
	_delay_ms(1000);

	flash(R,sc);
	_delay_ms(1000);
	
	flash(I,sc);
	_delay_ms(1000);

	flash(T,sc);
	_delay_ms(1000);

	flash(T,sc);
	_delay_ms(1000);
}

int main(void)
{
	_delay_ms(100);															//power can be weird when plugging in arduino, should help with this
	uint8_t sc = 0x07;														//define number of rows to be used
	init_spi();
	init_adc();
	SPI_data_send(shutdown, 0x01);											//disable shutdown mode
	SPI_set_brightness();
	SPI_clear_all(sc);														//clear any weird artifacts that might appear from resets
	while (1)
	{
		print_HI(sc);
		SPI_clear_all(sc);
		_delay_ms(500);
		print_BRITT(sc);
		SPI_clear_all(sc);
		_delay_ms(1000);
	}
}