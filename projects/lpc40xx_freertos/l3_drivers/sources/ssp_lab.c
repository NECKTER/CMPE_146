//
// Created by nick_ on 9/27/2020.
//

#include "ssp_lab.h"
#include "clock.h"
#include "gpio.h"
#include "lpc40xx.h"
#include <stdint.h>
#include <stdio.h>

void ssp2__init(uint32_t max_clock_mhz) {
  // Refer to LPC User manual and setup the register bits correctly

  //  LPC_IOCON->P1_0 &= ~(0x7); // ssp2 sck
  //  LPC_IOCON->P1_1 &= ~(0x7); // ssp2 mosi
  //  LPC_IOCON->P1_4 &= ~(0x7); // ssp2 miso
  //  LPC_IOCON->P1_0 |= (0x4);
  //  LPC_IOCON->P1_1 |= (0x4);
  //  LPC_IOCON->P1_4 |= (0x4);
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4);
  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4);
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4);

  // a) Power on Peripheral
  LPC_SC->PCONP |= (1 << 20); // bit 20 is ssp2
  // b) Setup control registers CR0 and CR1
  // page 610
  LPC_SSP2->CR0 = 0x7;      // 8 bit mode, SPI
  LPC_SSP2->CR1 = (1 << 1); // normal operation, ssp enabled
                            //  LPC_SSP2->CPSR = 96;
  // c) Setup prescalar register to be <= max_clock_mhz
  uint8_t divider = 2;
  const uint32_t cpu_clock_mhz = clock__get_core_clock_hz() / 1000000UL;
  while (max_clock_mhz < (cpu_clock_mhz / divider) && divider < 255) {
    divider += 2;
  }
  fprintf(stderr, "Clock = %d\nDivider: %d\nSCK: %d\n", cpu_clock_mhz, divider, cpu_clock_mhz / divider);
  LPC_SSP2->CPSR = divider;
}

uint8_t ssp2__swap_byte(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP2->DR = data_out; // set the data register 8bits
  while (LPC_SSP2->SR & (1 << 4)) {
    ;
  } // wait for bit 4 (busy signal)

  return (uint8_t)(LPC_SSP2->DR & 0xff); // return the data register
}