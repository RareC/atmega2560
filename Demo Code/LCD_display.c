/*###################################################################################################################
Created: 27/02/2018
Author: Chris
Description: Code to demonstrate functionality of LCD display driver
Hardware Pin Connections: D7 = PC3, D6 = PC2, D5 = PC1, D4 = PC0, RS = PG1, E = PG0
- R/W always held low for permanent write
- R/S low for commands, R/S high for data
- E active high to begin communication, must be toggled for each nibble of data being sent
####################################################################################################################*/ 
#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "LCDdisp.h"

int main(void)
{

	LCD_init();
	uint8_t lol [] = "short";
	LCD_move_curs(1,0);
	LCD_vintage_write(lol);
	LCD_scroll_right(lol,2);
    while (1) 
    {
		
    }
} 