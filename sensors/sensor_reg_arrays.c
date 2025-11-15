/*
 * Register array initialization implementation
 *
 * Copyright 2015-2024 Espressif Systems (Shanghai) PTE LTD
 * Licensed under the Apache License, Version 2.0
 */

#include "private_include/sensor_reg_arrays.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int sensor_reg_array_write16(uint8_t slv_addr, const uint16_t (*regs)[2])
{
    int i = 0;
    int ret = 0;

    while (!ret && regs[i][0] != SENSOR_REG_TERM) {
        if (regs[i][0] == SENSOR_REG_DELAY) {
            // Delay in milliseconds
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            // Write register
            ret = sensor_reg_write16(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }

    return ret;
}

int sensor_reg_array_write8(uint8_t slv_addr, const uint8_t (*regs)[2], size_t size)
{
    int i = 0;
    int ret = 0;

    while (!ret && i < size) {
        if (regs[i][0] == SENSOR_REG_DELAY_8) {
            // Delay in milliseconds
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            // Write register
            ret = sensor_reg_write(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }

    return ret;
}

int sensor_reg_array_write_regval(uint8_t slv_addr, const struct sensor_regval_list *regs)
{
    int i = 0;
    int ret = 0;

    // End marker is 0xFF for this format
    while (!ret && regs[i].reg_num != 0xFF) {
        ret = sensor_reg_write(slv_addr, regs[i].reg_num, regs[i].value);
        i++;
    }

    return ret;
}
