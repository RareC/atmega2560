/*
Chris Lyford 01/02/18
Hardware Used: Arduino atmega2560, max7219 + LED matrix module, 10k pot & resistor
IDE: Amtel studio 7
Compiler: avrispmkII
Pin connections: PB1 = SCK, PB2 = MOSI, PB4 = CS, PF0 analogue input

Could be way more readable with #define 's!
*/

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

void init_spi()
{
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1);          //enable SPI;set as master;set freq to 16MHz/64
	DDRB = 0b00010111;                  //set SCK as output; ~SS as output to control device
	PORTB |= (1<<0);                    //enable pull-up
}

void init_adc()
{
	ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);                //use internal 2.56 reference & left adjust result (can then ignore the two LSB's to store result in char!, use ADC0)
	ADCSRA |= (1<<ADEN)|(1 << ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);      //enable ADC, put into freerunning mode & set ADC freq prescaler = F_CPU/128
	ADCSRA |= (1<<ADSC);                //begin conversions
}

void SPI_data_send(char addr, char dat)
{
	PORTB &= ~(1<<4);                   //enable chip
	SPDR = addr;                      //load address into SPDR
	while(!(SPSR & (1<<SPIF) ));              //wait for transmission to finish
	SPDR = dat;
	while(!(SPSR & (1<<SPIF) ));
	PORTB |= 1<<4;                    //disable chip
}

void SPI_set_brightness()
{
	char pot = ADCH;
	pot = (unsigned char)pot >> 4;
	SPI_data_send(0x0A,pot);
}

void flash(char disp[7],char sc)
{
	SPI_data_send(0x0C,0x00);               //enter shutdown mode
	_delay_ms(300);
	SPI_data_send(0x0C,0x01);               //awaken again
	SPI_data_send(0x0B, sc);                //set row limit to whatever is defined by sc
	SPI_set_brightness();
	int i;
	for(i=0;i<sc;i++)
	{
		SPI_data_send((0x01+i),disp[i]);
	}
}

void print_HI(char sc)
{
	char H[7]={0x00,0x00,0x7E,0x08,0x08,0x7E,0x00};   //[0] is rightmost column and [7] leftmost, MSB is top row and LSB is bottom row
	char I[7]={0x00,0x00,0x00,0x00,0x5E,0x00,0x00};   //depends which way around you look at it of course!

	flash(H,sc);
	_delay_ms(1000);

	flash(I,sc);
	_delay_ms(1000);
}

void print_BRITT(char sc)
{
	char B[7]={0x00,0x00,0x2C,0x52,0x52,0x7E,0x00};
	char R[7]={0x00,0x00,0x20,0x20,0x3E,0x00,0x00};
	char I[7]={0x00,0x00,0x00,0x00,0x5E,0x00,0x00};
	char T[7]={0x00,0x00,0x00,0x12,0x7E,0x10,0x00};

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

void SPI_clear_all(char sc)
{
	SPI_data_send(0x0B, 0x08);              //enable all rows
	char row = 0x08;
	do
	{
		SPI_data_send(row,0x00);
	}while(row--);                         //clear all rows
	SPI_data_send(0x0B, sc);                //return to original scan range
}



int main(void)
{
	char sc = 0x07;                   //define number of rows to be used
	init_spi();
	init_adc();
	_delay_ms(10);
	SPI_data_send(0x0C, 0x01);              //disable shutdown mode
	SPI_set_brightness();
	SPI_clear_all(sc);
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