//
// Created by nick_ on 9/7/2020.
//
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t port;
  uint8_t pin;
} gpio_pin;

typedef enum {
  pin_led_0 = 18,
  pin_led_1 = 24,
  pin_led_2 = 26,
  pin_led_3 = 3,

  pin_sw_0 = 29,
  pin_sw_1 = 30,
  pin_sw_2 = 15,
  pin_sw_3 = 10,
} PINS;

typedef enum {
  port_led_0 = 1,
  port_led_1 = 1,
  port_led_2 = 1,
  port_led_3 = 2,

  port_sw_0 = 0,
  port_sw_1 = 0,
  port_sw_2 = 1,
  port_sw_3 = 1,
} PORTS;

/// Should alter the hardware registers to set the pin as input
void gpio0__set_as_input(gpio_pin pin);

/// Should alter the hardware registers to set the pin as output
void gpio0__set_as_output(gpio_pin pin);

/// Should alter the hardware registers to set the pin as high
void gpio0__set_high(gpio_pin pin);

/// Should alter the hardware registers to set the pin as low
void gpio0__set_low(gpio_pin pin);

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio0__set(gpio_pin pin, bool high);

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio0__get_level(gpio_pin pin);