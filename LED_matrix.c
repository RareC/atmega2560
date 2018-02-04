/*
Chris Lyford 01/02/18
Hardware Used: Arduino atmega2560, max7219 + LED matrix module, 10k pot & resistor
IDE: Amtel studio 7
Compiler: avrispmkII
Pin connections: PB1 = SCK, PB2 = MOSI, PB4 = CS, PF0 = analogue input
*/

#define F_CPU 16000000

#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <avr/io.h>
#include <util/delay.h>

void init_spi()
{
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1);									//enable SPI;set as master;set freq to 16MHz/64
	DDRB = 0b00010111;														//set SCK as output; ~SS as output to control device
	PORTB |= (1<<0);														//enable pull-up
}

void init_adc()
{
	ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);								//use internal 2.56V ref & left adjust result, use ADC0
	ADCSRA |= (1<<ADEN)|(1 << ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);      //enable ADC, put into free running mode & set ADC freq pre-scaler = F_CPU/128
	ADCSRA |= (1<<ADSC);													//begin conversions
}

void SPI_data_send(uint8_t addr, uint8_t dat)
{
	PORTB &= ~(1<<4);														//enable chip select
	SPDR = addr;															//load address into shift reg
	while(!(SPSR & (1<<SPIF) ));											//wait for transmission to finish
	SPDR = dat;
	while(!(SPSR & (1<<SPIF) ));
	PORTB |= 1<<4;															//disable chip select
}

void SPI_set_brightness()
{
	uint8_t pot = ADCH;														//read top 8 bits of ADC conversion
	pot = pot >> 4;															//only using top 4 bits as intensity reg only has 4, move to correct location
	SPI_data_send(brightness,pot);
}

void flash(uint8_t disp[7],uint8_t sc)
{
	SPI_data_send(shutdown,0x00);											//enter shutdown mode
	_delay_ms(300);
	SPI_data_send(shutdown,0x01);											//awaken again
	SPI_data_send(row_lim, sc);												//set row limit to whatever is defined by sc
	SPI_set_brightness();
	int i;
	for(i=0;i<sc;i++)
	{
		SPI_data_send((0x01+i),disp[i]);									//register for first row is 0x01, so add i to move to next row
	}
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

void SPI_clear_all(uint8_t sc)
{
	uint8_t row = 0x08;
	SPI_data_send(row_lim, row);											//enable all rows
	do
	{
		SPI_data_send(row,0x00);
	}while(row--);															//works backwards from reg 0x08 to 0x01 to clear all rows
	SPI_data_send(row_lim, sc);												//return to original scan range
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