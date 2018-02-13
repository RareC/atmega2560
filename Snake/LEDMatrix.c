#define F_CPU 16000000

#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <LEDMatrix.h>
#include <avr/io.h>
#include <util/delay.h>

void init_spi()
{
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR1);									//enable SPI;set as master;set freq to 16MHz/64
	DDRB = 0b00010111;														//set SCK as output; ~SS as output to control device
	PORTB |= (1<<0);														//enable pull-up
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