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

/* Since we are using 4 bit mode, data needs to be sent 4 bits at a time
The (E)nable pin needs to be toggled after each nibble being sent to signal there's new data*/
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

// The only difference between sending commands and data is the state of the RS pin 
void LCD_write_cmd(uint8_t command){
	PORTG &= ~RS;				//sending commands so set RS low
	LCD_send_data(command);	
}

void LCD_write_char(uint8_t character){
	PORTG |= RS;				//sending data so set RS high
	LCD_send_data(character);	
}

/* Words are sent as a sequence of chars
The DDRAM pointer is automatically incremented by 1 after each write so we only worry about sending chars */
void LCD_write_word(uint8_t *word){
	uint8_t i = 0;
	while(*(word+i)){
		LCD_write_char(*(word+i));
		i++;
	}
}

// Allows beginning writing at any position on the screen
void LCD_move_curs(uint8_t line, uint8_t offset){
	if (line == 1){
		LCD_write_cmd(0x80 + offset);	//set cursor to start of top line plus any character offset
	}
	else{
		LCD_write_cmd(0xC0 + offset);	//set cursor to start of bottom line plus any character offset
	}
}

/* Has a longer execution time than the other instructions so cannot use the default command function 
Since we don't poll the busy flag, we just wait the maximum amount of time instead*/
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

/*Useful if you only want to clear a single line and preserve the other
Has an execution time of ~600uS so might be faster to use this twice instead of clear display?*/
void LCD_clear_line(uint8_t line){		//clears a single line instead of the entire display
	LCD_move_curs(line,0);
	for (uint8_t i =0; i < 16; i++){
		LCD_write_char(0x11);
	}
}

//Displays text on the screen as if it's being types on an old terminal
void LCD_vintage_write(uint8_t *word){
	LCD_write_cmd(0x0F);				//turn on blinking cursor
	uint8_t i = 0;
	while(*(word+i)){
		LCD_write_char(*(word+i));
		i++;
		_delay_ms(SCROLL_SPEED);
	}
	LCD_write_cmd(0x0C);				//disable blinking cursor
}

//itoa only takes values <=int as argument so cannot use anything longer than 2 bytes on AVR 
void LCD_disp_num(int val){
	uint8_t buff [6];					//max value of 2 bytes has 5 digits plus 1 for sign
	itoa(val,buff,10);
	LCD_write_word(buff);
}

//Scrolls text from right to left across a given line
void LCD_scroll_right(uint8_t *word,uint8_t line){			
	uint8_t start_pos = 16;
	LCD_move_curs(line,start_pos-1);
	while(start_pos){
		LCD_write_word(word);
		LCD_clear_trail();
		start_pos--;
		LCD_move_curs(line,start_pos-1);
		_delay_ms(SCROLL_SPEED);
	}	
}

//For use by scroll functions, can't imagine a use outside of this
void LCD_clear_trail(){						//clear trailing characters with scrolling text
	for(uint8_t i = 0; i < 16; i++){		//Will not overflow onto other line regardless of cursor position so this covers everything
		LCD_write_char(0x11);
	}	
}

//LCD driver used is Hitachi chip set compatible so this follows their initialisation sequence
void LCD_init(){
	_delay_ms(15);
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