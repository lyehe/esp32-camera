/*
 * Register bit manipulation utilities
 *
 * Single Responsibility: Handle bit-level register operations
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

#ifndef __SENSOR_REG_BITS_H__
#define __SENSOR_REG_BITS_H__

#include <stdint.h>
#include "sensor_reg_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register bit manipulation
 *
 * This module provides utilities for manipulating individual bits in registers
 * without affecting other bits. Follows Single Responsibility Principle.
 */

/**
 * @brief Set specific bits in a register
 *
 * Read-modify-write operation to set specific bits while preserving others.
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param offset Bit offset (position of LSB)
 * @param mask Bit mask (before shifting)
 * @param value Value to set (before shifting and masking)
 * @return 0 on success, negative on error
 *
 * @example
 * // Set bits [4:2] to value 0b101 in register 0x10
 * sensor_reg_bits_set(addr, 0x10, 2, 0x07, 0x05);
 * // Before: xxxx_xxxx
 * // After:  xx10_1xxx (bits [4:2] = 101)
 */
int sensor_reg_bits_set(uint8_t slv_addr, uint16_t reg, uint8_t offset, uint8_t mask, uint8_t value);

/**
 * @brief Check if specific bit mask is set
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param mask Bit mask to check
 * @return 1 if all bits in mask are set, 0 otherwise, negative on error
 *
 * @example
 * // Check if bit 3 is set
 * sensor_reg_bits_check(addr, 0x10, 0x08);  // Returns 1 if bit 3 is 1
 */
int sensor_reg_bits_check(uint8_t slv_addr, uint16_t reg, uint8_t mask);

/**
 * @brief Clear specific bits in a register
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param mask Bits to clear (1 = clear, 0 = preserve)
 * @return 0 on success, negative on error
 */
static inline int sensor_reg_bits_clear(uint8_t slv_addr, uint16_t reg, uint8_t mask)
{
    int val = sensor_reg_read16(slv_addr, reg);
    if (val < 0) {
        return val;
    }

    return sensor_reg_write16(slv_addr, reg, (uint8_t)val & ~mask);
}

/**
 * @brief Set specific bits (without clearing others)
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param mask Bits to set (1 = set, 0 = preserve)
 * @return 0 on success, negative on error
 */
static inline int sensor_reg_bits_or(uint8_t slv_addr, uint16_t reg, uint8_t mask)
{
    int val = sensor_reg_read16(slv_addr, reg);
    if (val < 0) {
        return val;
    }

    return sensor_reg_write16(slv_addr, reg, (uint8_t)val | mask);
}

#ifdef __cplusplus
}
#endif

#endif // __SENSOR_REG_BITS_H__
