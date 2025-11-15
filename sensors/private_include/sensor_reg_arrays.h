/*
 * Register array initialization utilities
 *
 * Single Responsibility: Handle bulk register initialization from arrays
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

#ifndef __SENSOR_REG_ARRAYS_H__
#define __SENSOR_REG_ARRAYS_H__

#include <stdint.h>
#include <stdbool.h>
#include "sensor_reg_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register array initialization
 *
 * This module handles bulk register writes from arrays, with support for
 * different array formats and special markers (delays, end-of-list).
 * Follows Single Responsibility Principle.
 */

// Special register markers
#define SENSOR_REG_DELAY 0xFFFF    // Delay marker in 16-bit arrays
#define SENSOR_REG_TERM  0x0000    // Terminator marker in 16-bit arrays
#define SENSOR_REG_DELAY_8 0xFF    // Delay marker in 8-bit arrays

/**
 * @brief Write register array (16-bit addresses)
 *
 * Writes an array of [register, value] pairs. Supports:
 * - SENSOR_REG_DELAY: Delay in milliseconds (value field)
 * - SENSOR_REG_TERM: End of array marker
 *
 * @param slv_addr I2C slave address
 * @param regs Array of [reg, val] pairs (16-bit registers)
 * @return 0 on success, negative on error
 *
 * @example
 * static const uint16_t init_regs[][2] = {
 *     {0x1000, 0x42},
 *     {SENSOR_REG_DELAY, 10},  // Wait 10ms
 *     {0x1001, 0x43},
 *     {SENSOR_REG_TERM, 0x00}, // End marker
 * };
 * sensor_reg_array_write16(addr, init_regs);
 */
int sensor_reg_array_write16(uint8_t slv_addr, const uint16_t (*regs)[2]);

/**
 * @brief Write register array (8-bit addresses)
 *
 * For sensors with 8-bit register addresses. Requires explicit size.
 *
 * @param slv_addr I2C slave address
 * @param regs Array of [reg, val] pairs (8-bit registers)
 * @param size Number of pairs in array
 * @return 0 on success, negative on error
 *
 * @example
 * static const uint8_t init_regs[][2] = {
 *     {0x10, 0x42},
 *     {0xFF, 10},  // Delay 10ms
 *     {0x11, 0x43},
 * };
 * sensor_reg_array_write8(addr, init_regs, sizeof(init_regs)/(sizeof(uint8_t)*2));
 */
int sensor_reg_array_write8(uint8_t slv_addr, const uint8_t (*regs)[2], size_t size);

/**
 * @brief Write register array from regval_list struct (OV7670 format)
 *
 * Some sensors use a different struct format. This provides compatibility.
 *
 * @param slv_addr I2C slave address
 * @param regs Pointer to first element of regval_list array
 * @return 0 on success, negative on error
 */
struct sensor_regval_list {
    uint8_t reg_num;
    uint8_t value;
};

int sensor_reg_array_write_regval(uint8_t slv_addr, const struct sensor_regval_list *regs);

#ifdef __cplusplus
}
#endif

#endif // __SENSOR_REG_ARRAYS_H__
