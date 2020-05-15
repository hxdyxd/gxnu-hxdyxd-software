#ifndef _LCD1602_H
#define _LCD1602_H

#include "data_interface_hal.h"

extern char lcd_console_buffer[64];

#define lcd_printf(x,y,...)  do {\
snprintf(lcd_console_buffer, sizeof(lcd_console_buffer), __VA_ARGS__);\
lcd1602_write_string(x, y, lcd_console_buffer);\
}while(0)

void lcd1602_init(void);
void lcd1602_clear(void);
void lcd1602_write_char(uint8_t x, uint8_t y, char c);
void lcd1602_write_string(uint8_t x, uint8_t y, char *s);

#endif
