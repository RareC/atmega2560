#ifndef LEDMatrix
#define LEDMatrix

#define F_CPU 16000000

#define brightness 0x0A
#define row_lim 0x0B
#define shutdown 0x0C

#include <avr/io.h>
#include <util/delay.h>

void init_spi();
void SPI_data_send(uint8_t addr, uint8_t dat);
void SPI_set_brightness();
void flash(uint8_t disp[7],uint8_t sc);
void SPI_clear_all(uint8_t sc);

#endif // LEDMatrix