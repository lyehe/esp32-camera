/*
 * GalaxyCore sensor family common functions
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

#ifndef __GC_SENSOR_COMMON_H__
#define __GC_SENSOR_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration for GC sensor pixel format registers
 */
typedef struct {
    uint16_t output_format_reg;  // Register address for output format control
    uint8_t rgb565_value;         // Value for RGB565 format
    uint8_t yuv422_value;         // Value for YUV422 format
    uint8_t raw_value;            // Value for RAW Bayer format
    uint8_t grayscale_value;      // Value for Grayscale format (0 if not supported)
    bool use_full_write;          // true: write full byte, false: use set_reg_bits
    uint8_t bits_offset;          // Offset for set_reg_bits (if use_full_write is false)
    uint8_t bits_mask;            // Mask for set_reg_bits (if use_full_write is false)
} gc_pixformat_config_t;

/**
 * @brief Configuration for GC sensor mirror/flip registers
 */
typedef struct {
    uint16_t mirror_flip_reg;     // Register address for mirror/flip control
    uint8_t hmirror_bit;          // Bit position for horizontal mirror
    uint8_t vflip_bit;            // Bit position for vertical flip
} gc_mirror_config_t;

/**
 * @brief Set pixel format for GC sensors
 *
 * @param sensor Pointer to sensor structure
 * @param pixformat Desired pixel format
 * @param config Pointer to GC pixel format configuration
 * @return 0 on success, -1 on error
 */
int gc_set_pixformat(sensor_t *sensor, pixformat_t pixformat, const gc_pixformat_config_t *config);

/**
 * @brief Set horizontal mirror for GC sensors
 *
 * @param sensor Pointer to sensor structure
 * @param enable 1 to enable, 0 to disable
 * @param config Pointer to GC mirror configuration
 * @return 0 on success, negative on error
 */
int gc_set_hmirror(sensor_t *sensor, int enable, const gc_mirror_config_t *config);

/**
 * @brief Set vertical flip for GC sensors
 *
 * @param sensor Pointer to sensor structure
 * @param enable 1 to enable, 0 to disable
 * @param config Pointer to GC mirror configuration
 * @return 0 on success, negative on error
 */
int gc_set_vflip(sensor_t *sensor, int enable, const gc_mirror_config_t *config);

#ifdef __cplusplus
}
#endif

#endif // __GC_SENSOR_COMMON_H__
