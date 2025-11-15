/*
 * Common sensor helper functions implementation
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

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "private_include/sensor_common.h"

#define REG_DLY 0xffff
#define REGLIST_TAIL 0x0000

int sensor_check_reg_mask(uint8_t slv_addr, uint16_t reg, uint8_t mask)
{
    int val = sensor_read_reg(slv_addr, reg);
    if (val < 0) {
        return 0;
    }
    return (val & mask) == mask;
}

int sensor_set_reg_bits(uint8_t slv_addr, uint16_t reg, uint8_t offset, uint8_t mask, uint8_t value)
{
    int ret = sensor_read_reg(slv_addr, reg);
    if (ret < 0) {
        return ret;
    }

    uint8_t c_value = (uint8_t)ret;
    uint8_t new_value = (c_value & ~(mask << offset)) | ((value & mask) << offset);

    return sensor_write_reg(slv_addr, reg, new_value);
}

int sensor_write_regs_16bit(uint8_t slv_addr, const uint16_t (*regs)[2])
{
    int i = 0;
    int ret = 0;

    while (!ret && regs[i][0] != REGLIST_TAIL) {
        if (regs[i][0] == REG_DLY) {
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            ret = sensor_write_reg(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }

    return ret;
}

int sensor_write_regs_8bit(uint8_t slv_addr, const uint8_t (*regs)[2], size_t size)
{
    int i = 0;
    int ret = 0;

    while (!ret && i < size) {
        if (regs[i][0] == REG_DLY) {
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            ret = sensor_write_reg(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }

    return ret;
}
