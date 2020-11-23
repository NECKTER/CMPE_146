// @file gpio_isr.c
#include "gpio_isr.h"
// #include "gpio_lab.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdio.h>

// Note: You may want another separate array for falling vs. rising edge callbacks
static function_pointer_t gpio0_callbacks[32];
static function_pointer_t gpio2_callbacks[32]; // extend code for port 2

void gpio0__attach_interrupt(int pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  //   gpio0__set_as_input(pin);
  gpio__construct_as_input(GPIO__PORT_0, pin);
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
  } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
  }
  gpio0_callbacks[pin] = callback;
}

int pin_generated() {
  int pin = 0;
  while (!(LPC_GPIOINT->IO0IntStatF & (1 << pin)) && !(LPC_GPIOINT->IO0IntStatR & (1 << pin))) {
    pin++;
    if ((LPC_GPIOINT->IO0IntStatF & (1 << pin)) || (LPC_GPIOINT->IO0IntStatR & (1 << pin))) {
      break;
    }
    if (pin == 32) {
      break;
    }
  }
  return pin;
}

void gpio0__interrupt_dispatcher(void) {
  const int pin_that_generated_interrupt = pin_generated();
  function_pointer_t attached_user_handler = gpio0_callbacks[pin_that_generated_interrupt];
  attached_user_handler();
  LPC_GPIOINT->IO0IntClr |= (1 << pin_that_generated_interrupt);
}

void gpio2__attach_interrupt(int pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  // gpio2__set_as_input(pin);
  gpio__construct_as_input(GPIO__PORT_2, pin);
  if (interrupt_type == GPIO_INTR__FALLING_EDGE) {
    LPC_GPIOINT->IO2IntEnF |= (1 << pin);
  } else if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    LPC_GPIOINT->IO2IntEnR |= (1 << pin);
  }
  gpio2_callbacks[pin] = callback;
}

int pin_generated2() {
  int pin = 0;
  while (!(LPC_GPIOINT->IO2IntStatF & (1 << pin)) && !(LPC_GPIOINT->IO2IntStatR & (1 << pin))) {
    pin++;
    if ((LPC_GPIOINT->IO2IntStatF & (1 << pin)) || (LPC_GPIOINT->IO2IntStatR & (1 << pin))) {
      break;
    }
    if (pin == 32) {
      break;
    }
  }
  return pin;
}

void gpio2__interrupt_dispatcher(void) {
  const int pin_that_generated_interrupt = pin_generated2();
  function_pointer_t attached_user_handler = gpio2_callbacks[pin_that_generated_interrupt];
  attached_user_handler();
  LPC_GPIOINT->IO2IntClr |= (1 << pin_that_generated_interrupt);
}