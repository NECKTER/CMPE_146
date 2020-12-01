#include "i2c_slave_init.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "common_macros.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
void i2c2__slave_init(uint8_t slave_address_to_respond_to) {
  // LPC_I2C2->ADR0 |= (slave_address | (0x000000FF));
  //   LPC_I2C2->MASK2 |= 0x00;
  LPC_I2C2->ADR0 |= slave_address_to_respond_to;
  LPC_I2C2->CONSET |= 0x44;
  //   LPC_I2C0->ADR0 |= slave_address_to_respond_to;
  //   LPC_I2C1->ADR0 |= slave_address_to_respond_to;
  fprintf(stderr, "ADR0: %d\n", LPC_I2C2->ADR0);
}