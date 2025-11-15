/*
 * Register I/O abstraction layer
 *
 * Single Responsibility: Handle basic register read/write operations
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

#ifndef __SENSOR_REG_IO_H__
#define __SENSOR_REG_IO_H__

#include <stdint.h>
#include "sccb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register I/O abstraction
 *
 * This module provides a thin abstraction over SCCB (I2C) for sensor register access.
 * It follows the Dependency Inversion Principle by depending on the SCCB interface
 * rather than concrete I2C implementations.
 */

/**
 * @brief Read a sensor register (8-bit address)
 *
 * Inline for performance - reduces function call overhead for hot path
 *
 * @param slv_addr I2C slave address
 * @param reg Register address (8-bit)
 * @return Register value (0-255) on success, negative on error
 */
static inline int sensor_reg_read(uint8_t slv_addr, uint8_t reg)
{
    return SCCB_Read(slv_addr, reg);
}

/**
 * @brief Read a sensor register (16-bit address)
 *
 * @param slv_addr I2C slave address
 * @param reg Register address (16-bit)
 * @return Register value (0-255) on success, negative on error
 */
static inline int sensor_reg_read16(uint8_t slv_addr, uint16_t reg)
{
    return SCCB_Read(slv_addr, reg);
}

/**
 * @brief Write a sensor register (8-bit address)
 *
 * Inline for performance - reduces function call overhead for hot path
 *
 * @param slv_addr I2C slave address
 * @param reg Register address (8-bit)
 * @param value Value to write (8-bit)
 * @return 0 on success, negative on error
 */
static inline int sensor_reg_write(uint8_t slv_addr, uint8_t reg, uint8_t value)
{
    return SCCB_Write(slv_addr, reg, value);
}

/**
 * @brief Write a sensor register (16-bit address)
 *
 * @param slv_addr I2C slave address
 * @param reg Register address (16-bit)
 * @param value Value to write (8-bit)
 * @return 0 on success, negative on error
 */
static inline int sensor_reg_write16(uint8_t slv_addr, uint16_t reg, uint8_t value)
{
    return SCCB_Write(slv_addr, reg, value);
}

/**
 * @brief Write paged register (SmartSens SC family)
 *
 * SC sensors use paging mode: high byte written to 0xf0, then access low byte
 * This encapsulates the paging complexity.
 *
 * @param slv_addr I2C slave address
 * @param reg Full 16-bit register address
 * @param value Value to write
 * @return 0 on success, negative on error
 */
static inline int sensor_reg_write_paged(uint8_t slv_addr, uint16_t reg, uint8_t value)
{
    // Write page selector
    if (SCCB_Write(slv_addr, 0xf0, (reg >> 8) & 0xFF)) {
        return -1;
    }

    // Write to register
    return SCCB_Write(slv_addr, reg & 0xFF, value);
}

/**
 * @brief Read paged register (SmartSens SC family)
 *
 * @param slv_addr I2C slave address
 * @param reg Full 16-bit register address
 * @return Register value on success, negative on error
 */
static inline int sensor_reg_read_paged(uint8_t slv_addr, uint16_t reg)
{
    // Write page selector
    if (SCCB_Write(slv_addr, 0xf0, (reg >> 8) & 0xFF)) {
        return -1;
    }

    // Read from register
    return SCCB_Read(slv_addr, reg & 0xFF);
}

#ifdef __cplusplus
}
#endif

#endif // __SENSOR_REG_IO_H__
