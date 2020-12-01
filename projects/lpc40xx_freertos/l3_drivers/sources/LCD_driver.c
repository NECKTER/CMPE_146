#include "LCD_driver.h"
#include "delay.h"
#include <stdio.h>

static gpio_s register_select;   // green
static gpio_s read_write_select; // blue
// static gpio_s db_7; //yellow
// static gpio_s db_6; //red
// static gpio_s db_5;  //orange
// static gpio_s db_4; //grey
// static gpio_s db_3;
// static gpio_s db_2;
// static gpio_s db_1;
// static gpio_s db_0;
static gpio_s db[8];
static gpio_s lcd_enable; // white

static lcd_pin_init() {
  register_select = gpio__construct_as_output(GPIO__PORT_1, 28);
  read_write_select = gpio__construct_as_output(GPIO__PORT_4, 28);
  db[7] = gpio__construct_as_output(GPIO__PORT_2, 0);
  db[6] = gpio__construct_as_output(GPIO__PORT_2, 2);
  db[5] = gpio__construct_as_output(GPIO__PORT_2, 5);
  db[4] = gpio__construct_as_output(GPIO__PORT_2, 7);
  db[3] = gpio__construct_as_output(GPIO__PORT_2, 9);
  db[2] = gpio__construct_as_output(GPIO__PORT_0, 15);
  db[1] = gpio__construct_as_output(GPIO__PORT_0, 18);
  db[0] = gpio__construct_as_output(GPIO__PORT_0, 1);
  lcd_enable = gpio__construct_as_output(GPIO__PORT_0, 10);

  //   gpio__set_function(register_select, 0);
  //   gpio__set_function(read_write_select, 0);
  //   gpio__set_function(db[7], 0);
  //   gpio__set_function(db[6], 0);
  //   gpio__set_function(db[5], 0);
  //   gpio__set_function(db[4], 0);
  //   gpio__set_function(db[3], 0);
  //   gpio__set_function(db[2], 0);
  //   gpio__set_function(db[1], 0);
  //   gpio__set_function(db[0], 0);
  //   gpio__set_function(lcd_enable, 0);
}

static void set_function() {
  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__set(db[5]);
  gpio__set(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(10);
}

static void set_4_bit() {

  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__set(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

static void set_4_bit_two() {
  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__set(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);

  gpio__reset(lcd_enable);
  gpio__set(db[7]);
  gpio__set(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

static void clear_on_off() {
  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);

  gpio__reset(lcd_enable);
  gpio__set(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

static void clear_display() {
  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);

  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__set(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

static void entry_mode_set() {
  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);

  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__set(db[6]);
  gpio__set(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

static void set_on_off() {
  gpio__reset(lcd_enable);
  gpio__set(db[7]);
  gpio__reset(db[6]);
  gpio__reset(db[5]);
  gpio__reset(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);

  gpio__reset(lcd_enable);
  gpio__reset(db[7]);
  gpio__set(db[6]);
  gpio__set(db[5]);
  gpio__set(db[4]);
  gpio__set(lcd_enable);
  gpio__reset(lcd_enable);
  delay__ms(5);
}

// void lcd__write_string(const string16_t STRING, int line, uint8_t ADDRESS_offset, uint16_t DELAY_in_ms) {
//   uint8_t SET_DRAM_ADDRESS_INSTR = (1 << 7);
//   uint8_t FIRST_LINE_START_ADDRESS = 0x00, SECOND_LINE_START_ADDRESS = 0x40;

//   uint8_t INSTR_CODE = (line) ? (SET_DRAM_ADDRESS_INSTR | SECOND_LINE_START_ADDRESS)
//                               : (SET_DRAM_ADDRESS_INSTR | FIRST_LINE_START_ADDRESS);

//   //   lcd__send_instr_code(INSTR_CODE + ADDRESS_offset);

//   /* Note that string16_t has size of 17 b/c 1 extra space is added to accomodate NULL termination
//    * Subtract sizeof(string16_t) by 1 would give 16
//    * Subtract Address_offset to prevent out of boundary
//    */
//   for (uint8_t INDEX = 0; INDEX < (sizeof(string16_t) - 1 - ADDRESS_offset) && STRING[INDEX] != '\0'; INDEX++) {
//     if (DELAY_in_ms) // Don't call this function if delay is 0. Can cause minor delays and it's kinda noticable
//       delay__ms(DELAY_in_ms);
//     lcd__write_char(STRING[INDEX]);
//   }
// }

// void lcd__test_lcd(void) {
//   LCD_init();
//   puts("Initialization is DONE");
//   uint16_t DELAY = 100;
//   lcd__write_string("<", 0, 0, DELAY);
//   lcd__write_string(">", 0, 15, DELAY);
//   lcd__write_string("HELLO THERE", 0, 2, DELAY);
//   lcd__write_string("WORLD!", 0, 2 + 5 + 1, DELAY);
//   lcd__write_string("WELCOME TO CMPE", 1, 0, DELAY);
// }

void LCD_init() {

  lcd_pin_init();
  puts("lcd initialize");
  gpio__reset(register_select);
  //   gpio__reset(read_write_select); //always 0
  gpio__reset(lcd_enable);
  delay__ms(110);

  for (int i = 0; i < 3; i++) {
    set_function();
  }
  set_4_bit();
  set_4_bit_two();
  clear_on_off();
  clear_display();
  entry_mode_set();
  set_on_off();
  gpio__reset(lcd_enable);
  // 3h sent as instruction, then delay 4.1

  for (int i = 0; i < 8; i++) {
    gpio__reset(db[i]);
  }
  puts("lcd initialize finished");
}

void lcd_write(char ch) {
  uint8_t var = ch;
  gpio__set(register_select);
  for (int i = 0; i < 4; i++) {
    if (var & (0x1 << i) == 1) {
      gpio__set(db[4 + i]);
    } else {
      gpio__reset(db[4 + i]);
    }
  }

  delay__ms(10);
  gpio__set(lcd_enable);
  delay__ms(10);
  gpio__reset(lcd_enable);
  delay__ms(10);

  for (int i = 0; i < 4; i++) {
    if (var & (0x1 << i + 4) == 1) {
      gpio__set(db[4 + i]);
    } else {
      gpio__reset(db[4 + i]);
    }
  }

  delay__ms(10);
  gpio__set(lcd_enable);
  delay__ms(10);
  gpio__reset(lcd_enable);
  delay__ms(10);
}