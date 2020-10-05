//
// Created by nick_ on 9/27/2020.
//

#pragma once

#include <stdint.h>
#include <stdlib.h>

void ssp2__init(uint32_t max_clock_mhz);

uint8_t ssp2__swap_byte(uint8_t data_out);
