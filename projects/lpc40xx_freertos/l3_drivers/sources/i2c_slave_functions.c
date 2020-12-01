// static volatile uint8_t slave_memory[256];

// bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
//   // TODO: Read the data from slave_memory[memory_index] to *memory pointer
//   // TODO: return true if all is well (memory index is within bounds)
// }

// bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
//   // TODO: Write the memory_value at slave_memory[memory_index]
//   // TODO: return true if memory_index is within bounds
// }
#include "i2c_slave_functions.h"
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "common_macros.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"

static volatile uint8_t slave_memory_h[256];
// static volatile uint8_t slave_memory_read[256];

bool i2c_slave_callback__read_memory_h(uint8_t memory_index, uint8_t *memory) {
  // TODO: Read the data from slave_memory[memory_index] to *memory pointer
  // TODO: return true if all is well (memory index is within bounds)
  // printf("callback_read\n");
  *memory = slave_memory_h[memory_index];
  if (LPC_I2C2->STAT == 0x78 || LPC_I2C2->STAT == 0xB0) {
    // fprintf(stderr, "val: %d\n", *memory);
    // *memory = LPC_I2C2->DAT;
    return false;
  } else {
    *memory = slave_memory_h[memory_index];
    return true;
  }
}

bool i2c_slave_callback__write_memory_h(uint8_t memory_index, uint8_t memory_value) {
  // TODO: Write the memory_value at slave_memory[memory_index]
  // TODO: return true if memory_index is within bounds
  // printf("callback_write\n");
  slave_memory_h[memory_index] = memory_value;
  fprintf(stderr, "val: %d\n", memory_value);
  if (LPC_I2C2->STAT == 0x78 || LPC_I2C2->STAT == 0xB0) {

    // *memory = LPC_I2C2->DAT;
    return false;
  } else {
    return true;
  }
}