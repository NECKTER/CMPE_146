#include <stdint.h>

#include <stdio.h>

#include "uart_lab.h"

#include "gpio.h"

#include "lpc40xx.h"
#include "lpc_peripherals.h"

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {
  // Refer to LPC User manual and setup the register bits correctly
  // The first page of the UART chapter has good instructions
  // a) Power on Peripheral
  // b) Setup DLL, DLM, FDR, LCR registers
  if (uart == UART_2) {

    LPC_SC->PCONP |= (0x1 << 24);
    gpio__construct_with_function(GPIO__PORT_0, 10, GPIO__FUNCTION_2);
    gpio__construct_with_function(GPIO__PORT_0, 11, GPIO__FUNCTION_2);
    LPC_UART2->LCR |= (0x1 << 7);
    LPC_UART2->LCR |= (3 << 0);
    uint16_t div_baud = (((uint16_t)peripheral_clock) / (16 * baud_rate));
    LPC_UART2->DLM = (div_baud >> 8) & (0xFF);
    LPC_UART2->DLL = (div_baud >> 0) & (0xFF);
    LPC_UART2->LCR &= ~(0x1 << 7);
    LPC_UART2->FCR |= (0x1 << 0);
  } else if (uart == UART_3) {
    LPC_SC->PCONP |= (0x1 << 25);
    gpio__construct_with_function(GPIO__PORT_4, 28, GPIO__FUNCTION_2);
    gpio__construct_with_function(GPIO__PORT_4, 29, GPIO__FUNCTION_2);
    LPC_UART3->LCR |= (0x1 << 7);
    LPC_UART3->LCR |= (3 << 0);
    LPC_UART3->FCR |= (0x1 << 0);
    LPC_UART3->FDR |= (0x1 << 4);
    const float errors_fraction = 0.5;
    uint16_t div_baud = ((peripheral_clock) / (16 * baud_rate) + errors_fraction);
    LPC_UART3->DLM = (div_baud >> 8) & (0xFF);
    LPC_UART3->DLL = (div_baud >> 0) & (0xFF);
    LPC_UART3->LCR &= ~(1 << 7);
  }
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  // a) Check LSR for Receive Data Ready
  // b) Copy data from RBR register to input_byte
  if (uart == UART_2) {
    LPC_UART2->LCR &= ~(1 << 7);
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
    *input_byte = LPC_UART2->RBR & 0xFF;
    return true;
  } else if (uart == UART_3) {
    LPC_UART3->LCR &= ~(1 << 7);
    while (!(LPC_UART3->LSR & (1 << 0))) {
      ;
    }
    *input_byte = LPC_UART3->RBR & 0xFF;
    return true;
  }
  return false;
}
bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  // LPC_UART2->LCR &= ~(0x1 << 7);
  // a) Check LSR for Transmit Hold Register Empty
  // b) Copy output_byte to THR register
  if (uart == UART_2) {
    LPC_UART2->LCR &= ~(1 << 7);
    while (!(LPC_UART2->LSR & (0x1 << 5))) {
      ;
    }
    LPC_UART2->THR = output_byte;
    return true;
  } else if (uart == UART_3) {
    LPC_UART3->LCR &= ~(1 << 7);
    while (!(LPC_UART3->LSR & (1 << 5))) {
      ;
    }
    LPC_UART3->THR = output_byte;
    while (!(LPC_UART3->LSR & (1 << 5))) {
      ;
    }
    return true;
  }
  return false;
}