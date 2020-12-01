#pragma once

/**
 * Use memory_index and read the data to *memory pointer
 * return true if everything is well
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"
bool i2c_slave_callback__read_memory_h(uint8_t memory_index, uint8_t *memory);

/**
 * Use memory_index to write memory_value
 * return true if this write operation was valid
 */
bool i2c_slave_callback__write_memory_h(uint8_t memory_index, uint8_t memory_value);

// TODO: You can write the implementation of these functions in your main.c (i2c_slave_functionc.c is optional)