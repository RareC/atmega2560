/*
 * Snake.c
 *
 * Created: 12/02/2018 21:34:04
 * Author : Chris
 */ 
#define  F_CPU 16000000

#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "LEDMatrix.h"

//Enumerated data types - outside of main for ISR
enum direction {UP, DOWN, LEFT, RIGHT};	//makes turning more readable
enum direction dir = RIGHT;			//set default traveling direction

//isr variables
uint8_t has_changed = 0;			//prevents multiple direction changes per step

//Function Prototypes
void init_buttons();
void make_snake(uint8_t*,uint8_t*);
void move_snake(uint8_t*,uint8_t*,enum direction);
void store_moves(uint8_t*,uint8_t*,uint8_t);

ISR(INT0_vect)							//when right button pressed
{
	if(has_changed == 0)
	{
		has_changed = 1;
		switch(dir)
		{
			case UP:
			dir = RIGHT;
			break;
				
			case DOWN:
			dir = LEFT;
			break;
				
			case LEFT:
			dir = UP;
			break;
				
			case RIGHT:
			dir = DOWN;
			break;
		}
	}

}

ISR(INT2_vect)							//when left button pressed
{
	if(has_changed == 0)
	{
		has_changed = 1;
		switch(dir)
		{
			case UP:
			dir = LEFT;
			break;
		
			case DOWN:
			dir = RIGHT;
			break;
		
			case LEFT:
			dir = DOWN;
			break;
		
			case RIGHT:
			dir = UP;
			break;
		}
	}
}

int main(void)
{
	init_spi();
	init_buttons();
	uint8_t prev_rows[64];
	uint8_t prev_cols[64];				//history of previous locations head has been
	uint8_t *first_row_pos = prev_rows;	//pointer to first element of row history
	uint8_t *first_col_pos = prev_cols;	//pointer to first element of column history
	uint8_t grid[8][8];					//positions on matrix
	uint8_t length = 2;					//length of snake
	
	prev_cols[0] = 0x05;				//starting column
	prev_rows[0] = 0x02;				//starting row
	prev_cols[1] = 0x06;
	prev_rows[1] = 0x02;
	
	SPI_data_send(shutdown,0x01);		//disable shutdown mode
	SPI_clear_all(0x07);				//clear all rows and set scan range to all rows
	SPI_data_send(brightness,0x04);		//set brightness

    while (1) 
    {
		has_changed = 0;
		make_snake(first_col_pos, first_row_pos);
		store_moves(first_col_pos, first_row_pos,length);
		move_snake(first_col_pos,first_row_pos,dir);

    }
}

void init_buttons()
{
	//set flags
	sei();								//enable global interrupt
	EIMSK |= (1 << INT2) | (1 << INT0); //enable interrupt when PD0 or PD2 go low
	EICRA |= (1<< ISC01) | (1<< ISC21);//enable interrupt on falling edge									
}

void make_snake(uint8_t *col,uint8_t *row)
{
	SPI_clear_all(0x07);
	SPI_data_send(*col, *row);
	SPI_data_send(*(col+1),*(row+1));
	_delay_ms(1000);
}

 void move_snake(uint8_t *col,uint8_t *row,enum direction dir)
{
	cli();															//disable interrupts whilst dir is being used
	switch (dir)
	{
		case DOWN:
			if(*row>0x01)
			{
				*row = *row >> 1;
				sei();
				break;
			}
			//code to end game because boundary was hit
			sei();
			break;
		
		case UP:
			if(*row<0x80)
			{
				*row = *row << 1;
				sei();
				break;
			}
			//code to end game because boundary was hit
			sei();
			break;
		
		case LEFT:
			if(*col<0x08)									//do not move left if in leftmost column register
			{
				//SPI_data_send(*col,0x00);					//clear current row
				(*col)++;									//move to column on left
				sei();
				break;				
			}
			//code to end game because boundary was hit
			sei();
			break;
			
		case RIGHT:
			if(*col>0x01)
			{
				//SPI_data_send(*col,0x00); //clear current row
				(*col)--;
				sei();
				break;				
			}
			//code to end game because boundary was hit
			sei();
			break;
	}
}

void store_moves(uint8_t* col,uint8_t* row ,uint8_t len)
{
	for (int i = (len-1); i>0; i--)
	{
		*(col+i) = *(col+i-1);
	}
}
//Give it a tail!
