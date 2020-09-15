//
// Created by nick_ on 9/13/2020.
//

// @file gpio_isr.c
#include "gpio_isr.h"

#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // 1) Store the callback based on the pin at gpio0_callbacks
  gpio0_callbacks[pin] = callback;
  // 2) Configure GPIO 0 pin for rising or falling edge
  gpio_s sw2 = gpio__construct_as_input(0, pin);
  if (interrupt_type)                     // rising edge
    LPC_GPIOINT->IO0IntEnR |= (1 << pin); // enable inturupt
  else
    LPC_GPIOINT->IO0IntEnF |= (1 << pin); // enable inturupt
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "Dispatcher");
}

// We wrote some of the implementation for you
void gpio0__interrupt_dispatcher(void) {
  // Check which pin generated the interrupt
  const uint32_t pin_that_generated_interrupt = interruptPinDetection();
  function_pointer_t attached_user_handler = gpio0_callbacks[pin_that_generated_interrupt];

  // Invoke the user registered callback, and then clear the interrupt
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}

int interruptPinDetection() {
  uint32_t status = LPC_GPIOINT->IO0IntStatR | LPC_GPIOINT->IO0IntStatF;
  for (uint32_t i = 0; i < 32; ++i) {
    if (status & 0x1)
      return i;
    status = status >> 1;
  }
  return 0;
}

void clear_pin_interrupt(uint32_t pin) {
  LPC_GPIOINT->IO0IntClr |= (1 << pin); // clear inturupt
}