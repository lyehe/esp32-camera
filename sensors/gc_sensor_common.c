/*
 * GalaxyCore sensor family common functions implementation
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

#include "private_include/gc_sensor_common.h"
#include "private_include/sensor_common.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "gc_common";
#endif

// GC sensors use page 0xfe for page selection
#define GC_PAGE_SELECT_REG 0xfe
#define GC_PAGE_0 0x00

int gc_set_pixformat(sensor_t *sensor, pixformat_t pixformat, const gc_pixformat_config_t *config)
{
    int ret = 0;
    uint8_t format_value = 0;
    bool format_supported = true;

    // Select page 0
    ret = sensor_write_reg(sensor->slv_addr, GC_PAGE_SELECT_REG, GC_PAGE_0);
    if (ret < 0) {
        return ret;
    }

    // Determine format value based on pixel format
    switch (pixformat) {
    case PIXFORMAT_RGB565:
        format_value = config->rgb565_value;
        break;

    case PIXFORMAT_YUV422:
        format_value = config->yuv422_value;
        break;

    case PIXFORMAT_RAW:
        format_value = config->raw_value;
        break;

    case PIXFORMAT_GRAYSCALE:
        if (config->grayscale_value == 0) {
            ESP_LOGW(TAG, "Grayscale not supported");
            return -1;
        }
        format_value = config->grayscale_value;
        break;

    default:
        ESP_LOGW(TAG, "Unsupported format: %d", pixformat);
        format_supported = false;
        break;
    }

    if (!format_supported) {
        return -1;
    }

    // Write format value to register
    if (config->use_full_write) {
        ret = sensor_write_reg(sensor->slv_addr, config->output_format_reg, format_value);
    } else {
        ret = sensor_set_reg_bits(sensor->slv_addr, config->output_format_reg,
                                  config->bits_offset, config->bits_mask, format_value);
    }

    if (ret == 0) {
        sensor->pixformat = pixformat;
        ESP_LOGD(TAG, "Set pixformat to: %u", pixformat);
    }

    return ret;
}

int gc_set_hmirror(sensor_t *sensor, int enable, const gc_mirror_config_t *config)
{
    int ret = 0;

    // Select page 0
    ret = sensor_write_reg(sensor->slv_addr, GC_PAGE_SELECT_REG, GC_PAGE_0);
    if (ret < 0) {
        return ret;
    }

    // Set horizontal mirror bit
    ret = sensor_set_reg_bits(sensor->slv_addr, config->mirror_flip_reg,
                              config->hmirror_bit, 0x01, enable != 0);

    if (ret == 0) {
        sensor->status.hmirror = enable;
        ESP_LOGD(TAG, "Set h-mirror to: %d", enable);
    }

    return ret;
}

int gc_set_vflip(sensor_t *sensor, int enable, const gc_mirror_config_t *config)
{
    int ret = 0;

    // Select page 0
    ret = sensor_write_reg(sensor->slv_addr, GC_PAGE_SELECT_REG, GC_PAGE_0);
    if (ret < 0) {
        return ret;
    }

    // Set vertical flip bit
    ret = sensor_set_reg_bits(sensor->slv_addr, config->mirror_flip_reg,
                              config->vflip_bit, 0x01, enable != 0);

    if (ret == 0) {
        sensor->status.vflip = enable;
        ESP_LOGD(TAG, "Set v-flip to: %d", enable);
    }

    return ret;
}
