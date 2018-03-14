/*
 * lcd_display.c
 *
 * Created: 27/02/2018 18:47:10
 * Author : Chris
 D7 = PC3, D6 = PC2, D5 = PC1, D4 = PC0, RS = PG1, E = PG0
 R/W always held low for permanent write
 R/S low for commands, R/S high for data
 E active high to begin communication, must be toggled for each nibble of data being sent
 */ 
#define F_CPU 16000000

#define  RS 0x02
#define  E  0x01
#define  SCROLL_SPEED 300					//delay in ms between writing/moving characters

#include <avr/io.h>
#include <util/delay.h>


void LCD_send_data(uint8_t);
void LCD_write_cmd(uint8_t);
void LCD_write_char(uint8_t);
void LCD_write_word(uint8_t*);
void LCD_init();
void LCD_move_curs(uint8_t, uint8_t);
void LCD_clear_display();
void LCD_clear_line(uint8_t);
void LCD_clear_trail();
void LCD_scroll_right(uint8_t*,uint8_t);
//void LCD_scroll_left();
void LCD_disp_num(int);
void LCD_vintage_write(uint8_t*);

int main(void)
{
	DDRC |= 0x0F;						//set bottom half of port C as output
	DDRG |= 0x03;						//set bottom three bits of port G as output
	LCD_init();
	uint8_t lol [] = "short";
	LCD_move_curs(1,0);
	LCD_vintage_write(lol);
	LCD_scroll_right(lol,2);
    while (1) 
    {
		
    }
} 
void LCD_send_data(uint8_t data){
	PORTG |= E;
	PORTC = data >> 4;			//send top nibble
	PORTG &= ~E;
	_delay_us(37);
	PORTG |= E;
	PORTC = data;				//send lower nibble
	PORTG &= ~E;
	_delay_us(37);
}

void LCD_write_cmd(uint8_t command){
	PORTG &= ~RS;				//sending commands so set RS low
	LCD_send_data(command);	
}

void LCD_write_char(uint8_t character){
	PORTG |= RS;				//sending data so set RS high
	LCD_send_data(character);	
}

void LCD_write_word(uint8_t *word){
	uint8_t i = 0;
	while(*(word+i)){
		LCD_write_char(*(word+i));
		i++;
	}
}

void LCD_move_curs(uint8_t line, uint8_t offset){
	if (line == 1){
		LCD_write_cmd(0x80 + offset);	//set cursor to start of top line plus any character offset
	}
	else{
		LCD_write_cmd(0xC0 + offset);	//set cursor to start of bottom line plus any character offset
	}
}

void LCD_clear_display(){				//needs own function due to longer execution time
	PORTG &= ~RS;						//sending command so set RS low
	PORTG |= E;
	PORTC = 0x00;						//send top nibble
	PORTG &= ~E;
	_delay_ms(2);
	PORTG |= E;
	PORTC = 0x01;						//send lower nibble
	PORTG &= ~E;
	_delay_ms(2);
}

void LCD_clear_line(uint8_t line){		//clears a single line instead of the entire display
	LCD_move_curs(line,0);
	for (uint8_t i =0; i < 16; i++){
		LCD_write_char(0x11);
	}
}

void LCD_vintage_write(uint8_t *word){	//same as LCD_write_word but with cursor blinking like old style terminal
	LCD_write_cmd(0x0F);				//turn on blinking cursor
	uint8_t i = 0;
	while(*(word+i)){
		LCD_write_char(*(word+i));
		i++;
		_delay_ms(SCROLL_SPEED);
	}
	LCD_write_cmd(0x0C);				//disable blinking cursor
}

void LCD_disp_num(int val){
	uint8_t buff [6];					//max value of 2 bytes has 5 digits plus 1 for sign
	itoa(val,buff,10);
	LCD_write_word(buff);
}

void LCD_scroll_right(uint8_t *word,uint8_t line){			
	uint8_t start_pos = 16;
	LCD_move_curs(line,start_pos-1);
	while(start_pos){
		//LCD_clear_line(line);
		LCD_write_word(word);
		LCD_clear_trail();
		start_pos--;
		LCD_move_curs(line,start_pos-1);
		_delay_ms(SCROLL_SPEED);
	}	
}

void LCD_clear_trail(){						//clear trailing characters with scrolling text
	for(uint8_t i = 0; i < 16; i++){		//line 2 DRAM addresses will never be reached so okay
		LCD_write_char(0x11);
	}	
}

void LCD_init(){
	_delay_ms(15);						//wait as per spec
	PORTG &= ~E;						//ensure E low
	PORTG &= ~RS;						//sending commands so set RS low
	for(uint8_t i = 0; i < 3; i++){		//send function set command three times to initialise LCD
		PORTG |= E;
		PORTC = 0x03;					//Function set
		PORTG &= ~E;
		_delay_ms(5);					//wait 5ms
	}
	PORTG |= E;
	PORTC = 0x02;						//set 4-bit mode
	PORTG &= ~E;
	_delay_ms(1);
	
	LCD_clear_display();				//clear display
	LCD_write_cmd(0x28);				//Set number of lines to 2
	LCD_write_cmd(0x06);				//cursor move direction
	LCD_write_cmd(0x0C);				//disable cursor blink
}