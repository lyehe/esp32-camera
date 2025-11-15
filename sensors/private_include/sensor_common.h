/*
 * Common sensor helper functions
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SENSOR_COMMON_H__
#define __SENSOR_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include "sccb.h"
#include "sensor.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Inline register access functions for performance
 * These are used frequently and benefit from inlining
 */

/**
 * @brief Read a register value
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @return Register value on success, negative on error
 */
static inline int sensor_read_reg(uint8_t slv_addr, uint16_t reg)
{
    return SCCB_Read(slv_addr, reg);
}

/**
 * @brief Write a register value
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param value Value to write
 * @return 0 on success, negative on error
 */
static inline int sensor_write_reg(uint8_t slv_addr, uint16_t reg, uint8_t value)
{
    return SCCB_Write(slv_addr, reg, value);
}

/*
 * Common helper functions
 */

/**
 * @brief Check if register has specific bit mask set
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param mask Bit mask to check
 * @return 1 if mask is set, 0 otherwise
 */
int sensor_check_reg_mask(uint8_t slv_addr, uint16_t reg, uint8_t mask);

/**
 * @brief Set specific bits in a register
 *
 * @param slv_addr I2C slave address
 * @param reg Register address
 * @param offset Bit offset
 * @param mask Bit mask (before shifting)
 * @param value Value to set (before shifting and masking)
 * @return 0 on success, negative on error
 */
int sensor_set_reg_bits(uint8_t slv_addr, uint16_t reg, uint8_t offset, uint8_t mask, uint8_t value);

/**
 * @brief Write an array of register/value pairs
 *
 * @param slv_addr I2C slave address
 * @param regs Array of [register, value] pairs
 * @return 0 on success, negative on error
 */
int sensor_write_regs_16bit(uint8_t slv_addr, const uint16_t (*regs)[2]);

/**
 * @brief Write an array of register/value pairs (8-bit addresses)
 *
 * @param slv_addr I2C slave address
 * @param regs Array of [register, value] pairs
 * @param size Number of register pairs
 * @return 0 on success, negative on error
 */
int sensor_write_regs_8bit(uint8_t slv_addr, const uint8_t (*regs)[2], size_t size);

/*
 * Dummy/unsupported function stubs
 * Use these for sensor functions that are not implemented
 */

/**
 * @brief Dummy function for unsupported integer parameter operations
 */
static inline int sensor_unsupported_int(sensor_t *sensor, int val)
{
    ESP_LOGW("sensor", "Operation not supported");
    return -1;
}

/**
 * @brief Dummy function for unsupported gainceiling operations
 */
static inline int sensor_unsupported_gainceiling(sensor_t *sensor, gainceiling_t val)
{
    ESP_LOGW("sensor", "Gainceiling not supported");
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif // __SENSOR_COMMON_H__
