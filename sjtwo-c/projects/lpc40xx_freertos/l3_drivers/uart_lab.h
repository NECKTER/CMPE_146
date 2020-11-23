#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  UART_2,
  UART_3,
} uart_number_e;

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate); // initializes pins as UART

bool uart_lab__polled_get(uart_number_e uart, char *input_byte); // uses a polling algorithm to get a character

bool uart_lab__polled_put(uart_number_e uart,
                          char output_byte); // uses a polling algorithm to wait for a character to be available