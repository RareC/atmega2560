/*###################################################################################################################
Created: 12/02/2018
Author: Chris
Description: Source code of snake game using an 8x8 LED matrix
Hardware Pin Connections: PB1 = SCK, PB2 = MOSI, PB4 = CS, PF0 = analogue input
- ADC used to set brightness of LED's
####################################################################################################################*/ 
#define  F_CPU 16000000

#define min_speed 100
#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/wdt.h>
#include <LEDMatrix.h>

//Enumerated data types - outside of main for ISR
enum direction {UP, DOWN, LEFT, RIGHT};	//makes turning more readable
enum direction dir = RIGHT;			//set default traveling direction

//Global variables - global so ISR can use
uint8_t has_changed = 0;			//prevents multiple direction changes per step
uint8_t seed = 0;					//seed for random number gen

//Function Prototypes
void init_buttons();
void init_adc();
void make_snake(uint8_t*,uint8_t*,uint8_t*,uint8_t*);
void flash_food(uint8_t,uint8_t,uint8_t);
void move_snake(uint8_t*,uint8_t*,enum direction,uint8_t);
void store_moves(uint8_t*,uint8_t*,uint8_t);
void make_food(uint8_t*,uint8_t*,uint8_t*,uint8_t*);
uint8_t free_pos(uint8_t*,uint8_t*,uint8_t,uint8_t,uint8_t);
void clear_moves(uint8_t*,uint8_t*,uint8_t);
uint8_t rng();

ISR(INT0_vect)							//when right button pressed
{
	seed = TCNT0;						//set new seed for random number gen
	if(has_changed == 0){				//check if direction has already been modified this step
		has_changed = 1;				//direction has now been changed, prevent any more changes this step
		switch(dir){
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
	seed=TCNT0;						//set new seed for number gen
	if(has_changed == 0){				//check if direction has already been modified this step
		has_changed = 1;
		switch(dir){
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
	//main variables
	uint8_t prev_rows[64];
	uint8_t prev_cols[64];				//tracks previous positions
	uint8_t length = 1;					//length of snake
	uint8_t food_pos[2] = {0x07,0x08};	//stores food position and sets starting position
	
	//starting position
	prev_cols[0] = 0x05;				//starting column
	prev_rows[0] = 0x04;				//starting row
	
	//initialisation of hardware
	init_spi();
	init_adc();
	init_buttons();
	SPI_data_send(shutdown,0x01);		//disable shutdown mode
	SPI_clear_all(0x07);				//clear all rows and set scan range to all rows
	SPI_data_send(brightness,0x04);		//set brightness

    while (1) 
    {
		has_changed = 0;												//reset the check if direction has been modified
		make_snake(&prev_cols[0],&prev_rows[0],&food_pos[0],&length);	//displays snake/food on screen
		store_moves(&prev_cols[0],&prev_rows[0],length);				//shifts snake position along by 1 in array memory
		move_snake(&prev_cols[0],&prev_rows[0],dir,length);				//checks for walls and moves head of snake in direction

    }
}

void init_buttons(){
	//set interrupt options
	sei();								//enable global interrupt
	EIMSK |= (1 << INT2) | (1 << INT0); //enable interrupt when PD2 or PD0 go low
	EICRA |= (1<< ISC01) | (1<< ISC21);//enable interrupt on falling edge
	//set up timer0 for seed generation upon button press
	TCCR0B |= 1<<CS02;					//clk/256 pre-scaler for timer0										
}

void init_adc()
{
	ADMUX |= (1<<REFS1)|(1<<REFS0)|(1<<ADLAR);								//use internal 2.56V ref & left adjust result, use ADC0
	ADCSRA |= (1<<ADEN)|(1 << ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);      //enable ADC, put into free running mode & set ADC freq pre-scaler = F_CPU/128
	ADCSRA |= (1<<ADSC);													//begin conversions
}

void make_snake(uint8_t *col,uint8_t *row, uint8_t *food, uint8_t *len){
		
	SPI_clear_all(0x07);
	//iterate through col array and find entries matching 'i'
	//sum all rows for each col
	//send summed row data for each col
	uint8_t food_col;
	uint8_t row_no_food;
	uint8_t row_food;
	
	for(uint8_t i = 0x01; i<=0x08; i++){			//for each column
		uint8_t row_sum=0;							//resets the sum for each column
		for(uint8_t j=0; j<*len; j++){				//go through each element of the snake
			if(*(col+j) == i){						//if current element of array is in the column being checked
				row_sum += *(row+j);				//include in lights to be lit
			}
		}
		if((i == *food)){							//add food to correct column
			if((*col != *food) || (*row != *(food+1))){	//check that head of snake is not on food position
				food_col = i;				//get column with food in it
				row_no_food = row_sum;		//get row value before food addition
				row_sum += *(food+1);				//add light for food
				row_food = row_sum;			//row value with food
			}
			else{										//if the snake has just eaten the food
				make_food(food,col,row,len);
			}
		}
		SPI_data_send(i,row_sum);					//send column data 
	}
	flash_food(food_col,row_no_food,row_food);		//determine how long to wait before beginning next move and flash food
}

void flash_food(uint8_t col,uint8_t no_food,uint8_t with_food)									//set delay between steps and flash food light
{
	int pot= (ADCH << 1)+min_speed;					//read ADC pin, multiply by 2 and add minimum speed, needs to be int to store delays > 255!
	for(int i=0;i<pot;i++){
		_delay_ms(1);								
		if (i==pot/4){								//flash food LED on and off twice per step
			SPI_data_send(col,no_food);
		}
		else if (i==pot/2){							//only send data on exact values to improve speed
			SPI_data_send(col,with_food);
		}
		else if (i==(3*pot)/4){
			SPI_data_send(col,no_food);
		}
		else if (i==pot-1){
			SPI_data_send(col,with_food);
		}									
	}
}

void move_snake(uint8_t *col,uint8_t *row,enum direction dir, uint8_t len)
 {
	cli();														//disable interrupts whilst dir is being used
	uint8_t taken = 0;											//flag to determine if position being moved to is taken
	switch (dir){
		case DOWN:
			if(*row>0x01){
				*row = *row >> 1;
				taken = free_pos((col+1), (row+1), *col, *row, len); //check if head has tried to eat rest of snake
				if (taken){
					clear_moves(col,row,len);					//clear memory for next game - weird bug fix
					wdt_enable(WDTO_30MS);						//snake has eaten itself, reset game
					while(1);									//trap in loop so watchdog can trigger reset
				}
				break;
			}
			else{												//boundary has been hit, reset game
				clear_moves(col,row,len);
				wdt_enable(WDTO_30MS);							
				while(1);										//trap in loop so watchdog can trigger reset
			}
		
		case UP:
			if(*row<0x80){
				*row = *row << 1;
				taken = free_pos((col+1), (row+1), *col, *row, len);
				if (taken){
					clear_moves(col,row,len);
					wdt_enable(WDTO_30MS);						//snake has eaten itself, reset game
					while(1);									//trap in loop so watchdog can trigger reset
				}
				break;
			}
			else{												//boundary has been hit, reset game
				clear_moves(col,row,len);
				wdt_enable(WDTO_30MS);
				while(1);										//trap in loop so watchdog can trigger reset
			}													
		
		case LEFT:
			if(*col<0x08){										//do not move left if in leftmost column
				(*col)++;										//move to column on left
				taken = free_pos((col+1), (row+1), *col, *row, len);
				if (taken){
					clear_moves(col,row,len);
					wdt_enable(WDTO_30MS);						//snake has eaten itself, reset game
					while(1);									//trap in loop so watchdog can trigger reset
				}
				break;				
			}
			else{												//boundary has been hit, reset game
				clear_moves(col,row,len);
				wdt_enable(WDTO_30MS);
				while(1);
			}													//trap in loop so watchdog can trigger reset
			
		case RIGHT:
			if(*col>0x01){
				(*col)--;
				taken = free_pos((col+1), (row+1), *col, *row, len);
				if (taken){
					clear_moves(col,row,len);
					wdt_enable(WDTO_30MS);						//snake has eaten itself, reset game
					while(1);									//trap in loop so watchdog can trigger reset
				}
				break;				
			}
			else{												//boundary has been hit, reset game
				clear_moves(col,row,len);
				wdt_enable(WDTO_30MS);
				while(1);										//trap in loop so watchdog can trigger reset
			}
	}
	sei();														//re-enable interrupts once finished
}

void store_moves(uint8_t* col,uint8_t* row ,uint8_t len)
{
	for (int i = (len-1); i>0; i--){				//reverse iterate through array
		*(col+i) = *(col+i-1);						//shift values along by one
	}
	for (int i = (len-1); i>0; i--){
		*(row+i) = *(row+i-1);
	}
}

void make_food(uint8_t *food, uint8_t *col, uint8_t *row, uint8_t *len_ptr)
{
	(*len_ptr)++;									//increase length of snake
	if(*len_ptr == 64){
		wdt_enable(WDTO_30MS);						//YOU WIN! now do it again
		while(1);
	}
	*(col-1+*(len_ptr)) = 0;						//initialise new member of array to prevent null pointer
	*(row-1+*(len_ptr)) = 0;
	uint8_t pos_good = 1;
	
	while(pos_good){				
			uint8_t new_col = rng();				//start with random number
			new_col = new_col >> 5;					//shrink to three bit number
			if (new_col == 0){						//use result of 0 for last column 
				new_col = 0x08;
			}
				
			uint8_t new_row = rng();				//get random number			
			new_row /= 32;							//sort into range of 0-7
			switch(new_row)
			{
				case 0:
					new_row = 0x01;
					break;
				case 1:
					new_row = 0x02;
					break;
				case 2:
					new_row = 0x04;
					break;
				case 3:
					new_row = 0x08;
					break;
				case 4:
					new_row = 0x10;
					break;
				case 5:
					new_row = 0x20;
					break;
				case 6:
					new_row = 0x40;
					break;
				case 7:
					new_row = 0x80;
					break;
			}
					
			*food = new_col;						//set new food column
			*(food+1) = new_row;					//set new row according to MSB of new_row
			pos_good = free_pos(col,row,new_col,new_row,*(len_ptr));		//check if this position is already taken
	}
}

uint8_t free_pos(uint8_t *col, uint8_t *row, uint8_t check_col, uint8_t check_row, uint8_t len) 
{
	//iterate through used length of arr
	for (uint8_t i = 0; i < len; i++){
		if((*(col+i) == check_col) && (*(row+i) == check_row)){	//check if position is taken by snake
			return 1;
		}
	}
	return 0;
}

void clear_moves(uint8_t *col,uint8_t *row,uint8_t len)			//weird bug occurs upon restarting the game when dying with 2 or more length
{																
	for(int i=0;i<len;i++){										//clear every element of the array
		*(col+i) = 0;											//stops game remembering last position from previous game to count as a loss
		*(row+i) = 0;											
	}
}

uint8_t rng() {										//random number gen modified from:http://engineeringnotes.blogspot.co.uk/2015/07/a-fast-random-function-for-arduinoc.html
	static uint8_t y = 0;										//keeps value of y between cycles 
	y += seed;
	y ^= y << 2; 
	y ^= y >> 7; 
	y ^= y << 7;
	return (y);
}

//todo: 
//add LCD display for user interface
//starting and pausing game
//score tracking, keep highest score in EEPROM to be recalled