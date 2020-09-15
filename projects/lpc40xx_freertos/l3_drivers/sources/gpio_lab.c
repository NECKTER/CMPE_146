//
// Created by nick_ on 9/7/2020.
//
#include "gpio_lab.h"

#include "lpc40xx.h"

static LPC_GPIO_TypeDef *gpio__get_struct(gpio_pin pin) { return (LPC_GPIO_TypeDef *)(LPC_GPIO0 + pin.port); }

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(gpio_pin pin) {
  LPC_GPIO_TypeDef *gpio = gpio__get_struct(pin);
  const uint32_t p = (1 << pin.pin);

  gpio->DIR &= ~p;
}

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(gpio_pin pin) {
  LPC_GPIO_TypeDef *gpio = gpio__get_struct(pin);
  const uint32_t p = (1 << pin.pin);

  gpio->DIR |= p;
}

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(gpio_pin pin) {
  LPC_GPIO_TypeDef *gpio = gpio__get_struct(pin);
  const uint32_t p = (1 << pin.pin);

  gpio->SET = p;
}

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(gpio_pin pin) {
  LPC_GPIO_TypeDef *gpio = gpio__get_struct(pin);
  const uint32_t p = (1 << pin.pin);

  gpio->CLR = p;
}

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio0__set(gpio_pin pin, bool high) {
  if (high)
    gpio0__set_high(pin);
  else
    gpio0__set_low(pin);
}

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(gpio_pin pin) {
  LPC_GPIO_TypeDef *gpio = gpio__get_struct(pin);
  const uint32_t p = (1 << pin.pin);

  uint32_t state = gpio->PIN & p;
  //  printf("port %d pin %d mask%x state%x \n", pin.port, pin.pin, p, gpio->PIN);
  return (state >> pin.pin);
}