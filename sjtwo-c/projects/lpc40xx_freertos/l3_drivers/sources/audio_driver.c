#include "audio_driver.h"
// #include "gpio.h"
// #include "lpc40xx.h"
// #include "lpc_peripherals.h"
// #include <stdio.h>

static gpio_s xdcs_pin_h;
static gpio_s dreq_pin_h;
static gpio_s adesto_cs_h;
static gpio_s rst_pin_h;
static gpio_s xdcs_data_select_h;

static int rst;
static bool pause_music_var = false;

void pin_config() {
  rst = 0;
  xdcs_pin_h = gpio__construct_as_output(GPIO__PORT_0, 25);
  dreq_pin_h = gpio__construct_as_input(GPIO__PORT_1, 20);
  rst_pin_h = gpio__construct_as_output(GPIO__PORT_2, 1);
  xdcs_data_select_h = gpio__construct_as_output(GPIO__PORT_0, 26);
}

void reset() {
  if (rst >= 0) {
    gpio__reset(rst_pin_h);
    delay__ms(100);
    gpio__set(rst_pin_h);
  }

  gpio__set(xdcs_pin_h);
  delay__ms(100);
  gpio__set(xdcs_data_select_h);
  delay__ms(100);
  soft_reset();
  delay__ms(100);

  sci_write(0x03, 0x6000);
}

uint8_t begin() {
  pin_config();
  if (reset >= 0) {
    gpio__reset(rst_pin_h); // set rst as low
  }

  delay__ms(5);
  gpio__set(xdcs_pin_h);         // set cs as high
  gpio__set(xdcs_data_select_h); // set cs as high

  // ssp2__initialize(24);
  reset();
  sci_write(0x02, 0x050F);
  set_volume(0x70, 0x08);
}

void play_data(uint8_t *buffer, uint16_t buffer_size) {
  gpio__reset(xdcs_data_select_h);
  for (int i = 0; i < buffer_size; i = i + 32) {
    while (!dreq_level()) {
      ;
    }
    for (int j = 0; j < 32; j++) {
      ssp2__exchange_byte(*buffer++);
    }
  }
  gpio__set(xdcs_data_select_h);
}

// void reset();

void soft_reset() {
  sci_write(0x00, 0x0800 | 0x0004);
  delay__ms(100);
}

bool pause_music(bool pause) {
  if (pause) {
    return true;
  } else {
    return false;
  }
}

static void decoder_xcs_h() {
  // LPC_GPIO1->CLR |= (0x1 << 10);
  // gpio__reset(adesto_cs);
  gpio__reset(xdcs_pin_h);
}

static void decoder_xds_h() {
  // LPC_GPIO1->SET |= (0x1 << 10);
  // gpio__set(adesto_cs);
  gpio__set(xdcs_pin_h);
}

bool dreq_level() {
  if (gpio__get(dreq_pin_h)) {
    return true;
  } else {
    return false;
  }
}

uint16_t sci_read(uint8_t address_to_read_from) {
  uint16_t ret;
  uint8_t val1;
  uint8_t val2;
  decoder_xcs_h();
  {
    uint8_t byte_FF = ssp2__exchange_byte(0x03);
    ssp2__exchange_byte(address_to_read_from);
    val1 = ssp2__exchange_byte(0x0A);
    val2 = ssp2__exchange_byte(0x0A);
  }
  decoder_xds_h();
  ret = val1;
  ret = ret << 8;
  ret |= val2;
  return ret;
}

void sci_write(uint8_t address_to_write_to, uint16_t data_to_write) {
  uint8_t lower = data_to_write & 0xFF;
  uint8_t higher = data_to_write >> 8;
  decoder_xcs_h();
  {
    ssp2__exchange_byte(0x02);
    ssp2__exchange_byte(address_to_write_to);
    ssp2__exchange_byte(higher);
    ssp2__exchange_byte(lower);
  }
  decoder_xds_h();
}

void sine_test(uint8_t which_sine_test, uint16_t ms_delay) {
  reset();
  // uint16_t ch = sci_read(0x00);
  // fprintf(stderr, "MODE: %04x\n", ch);

  uint16_t read_mode_reg = sci_read(0x00);
  delay__ms(10);
  read_mode_reg = sci_read(0x00);
  // fprintf(stderr, "MODE: %04x\n", read_mode_reg);
  read_mode_reg |= 0x0020;
  sci_write(0x00, read_mode_reg);
  // sci_write(0x0B, 0x703F);
  set_volume(0x70, 0x3F);

  while (!dreq_level()) {
    ;
  }

  gpio__reset(xdcs_data_select_h);
  // if (!gpio__get(xdcs_data_select_h)) {
  //   fprintf(stderr, "low");
  // }
  delay__ms(10);
  ssp2__exchange_byte(0x53);
  ssp2__exchange_byte(0xEF);
  ssp2__exchange_byte(0x6E);
  ssp2__exchange_byte(which_sine_test);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  delay__ms(1000);
  gpio__set(xdcs_data_select_h);
  // if (gpio__get(xdcs_data_select_h)) {
  //   fprintf(stderr, "high");
  // }
  fprintf(stderr, "delay ins: %d\n", ms_delay);
  delay__ms(ms_delay);
  // fprintf(stderr, "second delay\n");
  gpio__reset(xdcs_data_select_h);
  delay__ms(10);
  ssp2__exchange_byte(0x45);
  ssp2__exchange_byte(0x78);
  ssp2__exchange_byte(0x69);
  ssp2__exchange_byte(0x74);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  ssp2__exchange_byte(0x00);
  delay__ms(10);
  gpio__set(xdcs_data_select_h);
  // fprintf(stderr, "1");
}

void set_volume(uint8_t left, uint8_t right) {
  uint16_t vol = right | (left << 8);
  sci_write(0x0B, vol);
  // sci_write(0x)
}

// void spi_write_data(uint8_t data_to_write);

// void spi_write_buffer(uint8_t *char_pointer_to_buffer, uint16_t number_of_elements_in_buffer);

// uint8_t spi_read();

// void set_volume_hard_code(uint8_t left, uint8_t right);

// void print_registers();

// void play_data(uint8_t *buffer, uint8_t buffer_size);

// bool ready_for_data();
