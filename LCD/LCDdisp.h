/* Header file for Hitachi compatible LCD displays
D7 = PC3, D6 = PC2, D5 = PC1, D4 = PC0, RS = PG1, E = PG0
R/W always held low for permanent write
R/S low for commands, R/S high for data
E active high to begin communication, must be toggled for each nibble of data being sent */

#ifndef LCDdisp
#define LCDdisp

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

#endif 