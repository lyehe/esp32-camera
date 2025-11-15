/*
 * Register bit manipulation implementation
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

#include "private_include/sensor_reg_bits.h"

int sensor_reg_bits_set(uint8_t slv_addr, uint16_t reg, uint8_t offset, uint8_t mask, uint8_t value)
{
    // Read current value
    int ret = sensor_reg_read16(slv_addr, reg);
    if (ret < 0) {
        return ret;
    }

    // Calculate new value
    uint8_t current = (uint8_t)ret;
    uint8_t new_value = (current & ~(mask << offset)) | ((value & mask) << offset);

    // Write back
    return sensor_reg_write16(slv_addr, reg, new_value);
}

int sensor_reg_bits_check(uint8_t slv_addr, uint16_t reg, uint8_t mask)
{
    int val = sensor_reg_read16(slv_addr, reg);
    if (val < 0) {
        return 0;  // Treat read errors as "not set"
    }

    return ((uint8_t)val & mask) == mask ? 1 : 0;
}
