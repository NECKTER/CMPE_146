#pragma once

#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

typedef char string16_t[16 + 1];

void LCD_init();

void lcd_write(char ch);

// void lcd__write_string(const string16_t STRING, int line, uint8_t ADDRESS_offset, uint16_t DELAY_in_ms);

// void lcd__test_lcd(void);