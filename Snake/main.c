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

enum direction {UP, DOWN, LEFT, RIGHT};	//makes turning more readable

#include <avr/io.h>
#include <util/delay.h>
#include "LEDMatrix.h"

//Function Prototypes
void make_snake(uint8_t,uint8_t);
void move_snake(uint8_t*,uint8_t*,enum direction);



void disp(uint8_t sc, uint8_t dat);

int main(void)
{
	init_spi();
	
	uint8_t prev_rows[64];
	uint8_t prev_cols[64];				//history of previous locations head has been
	uint8_t *first_row_pos = prev_rows;	//pointer to first element of row history
	uint8_t *first_col_pos = prev_cols;	//pointer to first element of column history
	uint8_t grid[8][8];					//positions on matrix
	uint8_t length = 1;					//length of snake
	enum direction dir = RIGHT;			//set default traveling direction	
	
	prev_cols[0] = 0x04;				//starting column
	prev_rows[0] = 0x02;				//starting row
	
	SPI_data_send(shutdown,0x01);		//disable shutdown mode
	SPI_clear_all(0x07);				//clear all rows and set scan range to all rows
	SPI_data_send(brightness,0x04);		//set brightness

    while (1) 
    {
		make_snake(prev_cols[0], prev_rows[0]);
		move_snake(first_col_pos,first_row_pos,dir);
    }
}

void make_snake(uint8_t col,uint8_t row)
{
	SPI_data_send(col, row);
	_delay_ms(1000);
}

 void move_snake(uint8_t *col,uint8_t *row,enum direction dir)
{
	switch (dir)
	{
		case DOWN:
			if(*row>0x01)
			{
				*row = *row >> 1;
				break;
			}
			//code to end game because boundary was hit
			break;
		
		case UP:
			if(*row<0x80)
			{
				*row = *row << 1;
				break;
			}
			//code to end game because boundary was hit
			break;
		
		case LEFT:
			if(*col<0x08)									//do not move left if in leftmost column register
			{
				SPI_data_send(*col,0x00);					//clear current row
				*col += 1;									//move to column on left
				break;				
			}
			//code to end game because boundary was hit
			break;
			
		case RIGHT:
			if(*col>0x01)
			{
				SPI_data_send(*col,0x00); //clear current row
				*col -= 1;
				break;				
			}
			//code to end game because boundary was hit
			break;
	}
}
//Make it turn!
//Give it a tail!
