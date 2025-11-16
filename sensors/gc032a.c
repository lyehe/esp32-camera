// Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sccb.h"
#include "gc032a.h"
#include "gc032a_regs.h"
#include "gc032a_settings.h"
#include "private_include/sensor_common.h"
#include "private_include/gc_sensor_common.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "gc032a";
#endif

#define H8(v) ((v)>>8)
#define L8(v) ((v)&0xff)

// GC032A pixel format configuration
static const gc_pixformat_config_t gc032a_pixformat_config = {
    .output_format_reg = 0x44,
    .rgb565_value = 6,
    .yuv422_value = 3,
    .raw_value = 0,
    .grayscale_value = 0,  // Not supported
    .use_full_write = false,
    .bits_offset = 0,
    .bits_mask = 0x1f,
};

// GC032A mirror/flip configuration
static const gc_mirror_config_t gc032a_mirror_config = {
    .mirror_flip_reg = P0_CISCTL_MODE1,  // 0x17
    .hmirror_bit = 0,
    .vflip_bit = 1,
};

// Debug print helper (sensor-specific, uses sensor_common functions)
static void print_regs(uint8_t slv_addr)
{
#ifdef DEBUG_PRINT_REG
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG, "REG list look ======================");
    for (size_t i = 0xf0; i <= 0xfe; i++) {
        ESP_LOGI(TAG, "reg[0x%02x] = 0x%02x", i, sensor_read_reg(slv_addr, i));
    }
    ESP_LOGI(TAG, "\npage 0 ===");
    sensor_write_reg(slv_addr, 0xfe, 0x00); // page 0
    for (size_t i = 0x03; i <= 0x24; i++) {
        ESP_LOGI(TAG, "p0 reg[0x%02x] = 0x%02x", i, sensor_read_reg(slv_addr, i));
    }
    for (size_t i = 0x40; i <= 0x95; i++) {
        ESP_LOGI(TAG, "p0 reg[0x%02x] = 0x%02x", i, sensor_read_reg(slv_addr, i));
    }
    ESP_LOGI(TAG, "\npage 3 ===");
    sensor_write_reg(slv_addr, 0xfe, 0x03); // page 3
    for (size_t i = 0x01; i <= 0x43; i++) {
        ESP_LOGI(TAG, "p3 reg[0x%02x] = 0x%02x", i, sensor_read_reg(slv_addr, i));
    }
#endif
}

// Register array write helper with delay support (sensor-specific)
static int write_regs(uint8_t slv_addr, const uint16_t (*regs)[2])
{
    int i = 0, ret = 0;
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

static int reset(sensor_t *sensor)
{
    int ret;
    // Software Reset: clear all registers and reset them to their default values
    ret = sensor_write_reg(sensor->slv_addr, RESET_RELATED, 0xf0);
    if (ret) {
        ESP_LOGE(TAG, "Software Reset FAILED!");
        return ret;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

    ret = write_regs(sensor->slv_addr, gc032a_default_regs);
    if (ret == 0) {
        ESP_LOGD(TAG, "Camera defaults loaded");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        sensor_write_reg(sensor->slv_addr, 0xfe, 0x00);
        sensor_set_reg_bits(sensor->slv_addr, 0xf7, 1, 0x01, 1); // PLL_mode1:div2en
        sensor_set_reg_bits(sensor->slv_addr, 0xf7, 7, 0x01, 1); // PLL_mode1:dvp mode
        sensor_set_reg_bits(sensor->slv_addr, 0xf8, 0, 0x3f, 8); //PLL_mode2 :divx4
        sensor_set_reg_bits(sensor->slv_addr, 0xfa, 4, 0x0f, 2); //vlk div mode :divide_by
    }

    return ret;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    return gc_set_pixformat(sensor, pixformat, &gc032a_pixformat_config);
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    ESP_LOGI(TAG, "set_framesize");
    int ret = 0;
    if (framesize > FRAMESIZE_VGA) {
        ESP_LOGW(TAG, "Invalid framesize: %u", framesize);
        framesize = FRAMESIZE_VGA;
    }
    sensor->status.framesize = framesize;
    uint16_t w = resolution[framesize].width;
    uint16_t h = resolution[framesize].height;
    uint16_t row_s = (resolution[FRAMESIZE_VGA].height - h) / 2;
    uint16_t col_s = (resolution[FRAMESIZE_VGA].width - w) / 2;

    sensor_write_reg(sensor->slv_addr, 0xfe, 0x00);
    sensor_write_reg(sensor->slv_addr, P0_ROW_START_HIGH, H8(row_s)); // Row_start[8]
    sensor_write_reg(sensor->slv_addr, P0_ROW_START_LOW, L8(row_s)); // Row_start[7:0]
    sensor_write_reg(sensor->slv_addr, P0_COLUMN_START_HIGH, H8(col_s)); // Column_start[9:8]
    sensor_write_reg(sensor->slv_addr, P0_COLUMN_START_LOW, L8(col_s)); // Column_start[7:0]
    sensor_write_reg(sensor->slv_addr, P0_WINDOW_HEIGHT_HIGH, H8(h + 8)); //window_height [8]
    sensor_write_reg(sensor->slv_addr, P0_WINDOW_HEIGHT_LOW, L8(h + 8)); //window_height [7:0]
    sensor_write_reg(sensor->slv_addr, P0_WINDOW_WIDTH_HIGH, H8(w + 8)); //window_width [9:8]
    sensor_write_reg(sensor->slv_addr, P0_WINDOW_WIDTH_LOW, L8(w + 8)); //window_width [7:0]

    sensor_write_reg(sensor->slv_addr, P0_WIN_MODE, 0x01);
    sensor_write_reg(sensor->slv_addr, P0_OUT_WIN_HEIGHT_HIGH, H8(h));
    sensor_write_reg(sensor->slv_addr, P0_OUT_WIN_HEIGHT_LOW, L8(h));
    sensor_write_reg(sensor->slv_addr, P0_OUT_WIN_WIDTH_HIGH, H8(w));
    sensor_write_reg(sensor->slv_addr, P0_OUT_WIN_WIDTH_LOW, L8(w));

    if (ret == 0) {
        ESP_LOGD(TAG, "Set framesize to: %ux%u", w, h);
    }
    print_regs(sensor->slv_addr);
    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    return gc_set_hmirror(sensor, enable, &gc032a_mirror_config);
}

static int set_vflip(sensor_t *sensor, int enable)
{
    return gc_set_vflip(sensor, enable, &gc032a_mirror_config);
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    int ret = 0;
    ret = sensor_write_reg(sensor->slv_addr, 0xfe, 0x00);
    ret |= sensor_set_reg_bits(sensor->slv_addr, P0_DEBUG_MODE2, 3, 0x01, enable);
    if (ret == 0) {
        sensor->status.colorbar = enable;
        ESP_LOGD(TAG, "Set colorbar to: %d", enable);
    }
    return ret;
}

static int get_reg(sensor_t *sensor, int reg, int mask)
{
    int ret = 0;
    if (mask > 0xFF) {
        ESP_LOGE(TAG, "mask should not more than 0xff");
    } else {
        ret = sensor_read_reg(sensor->slv_addr, reg);
    }
    if (ret > 0) {
        ret &= mask;
    }
    return ret;
}

static int set_reg(sensor_t *sensor, int reg, int mask, int value)
{
    int ret = 0;
    if (mask > 0xFF) {
        ESP_LOGE(TAG, "mask should not more than 0xff");
    } else {
        ret = sensor_read_reg(sensor->slv_addr, reg);
    }
    if (ret < 0) {
        return ret;
    }
    value = (ret & ~mask) | (value & mask);

    if (mask > 0xFF) {

    } else {
        ret = sensor_write_reg(sensor->slv_addr, reg, value);
    }
    return ret;
}

static int init_status(sensor_t *sensor)
{
    sensor_write_reg(sensor->slv_addr, 0xfe, 0x00);
    sensor->status.brightness = 0;
    sensor->status.contrast = 0;
    sensor->status.saturation = 0;
    sensor->status.sharpness = 0;
    sensor->status.denoise = 0;
    sensor->status.ae_level = 0;
    sensor->status.gainceiling = 0;
    sensor->status.awb = 0;
    sensor->status.dcw = 0;
    sensor->status.agc = 0;
    sensor->status.aec = 0;
    sensor->status.hmirror = sensor_check_reg_mask(sensor->slv_addr, P0_CISCTL_MODE1, 0x01);
    sensor->status.vflip = sensor_check_reg_mask(sensor->slv_addr, P0_CISCTL_MODE1, 0x02);
    sensor->status.colorbar = 0;
    sensor->status.bpc = 0;
    sensor->status.wpc = 0;
    sensor->status.raw_gma = 0;
    sensor->status.lenc = 0;
    sensor->status.quality = 0;
    sensor->status.special_effect = 0;
    sensor->status.wb_mode = 0;
    sensor->status.awb_gain = 0;
    sensor->status.agc_gain = 0;
    sensor->status.aec_value = 0;
    sensor->status.aec2 = 0;
    return 0;
}

static int set_dummy(sensor_t *sensor, int val)
{
    ESP_LOGW(TAG, "Unsupported");
    return -1;
}
static int set_gainceiling_dummy(sensor_t *sensor, gainceiling_t val)
{
    ESP_LOGW(TAG, "Unsupported");
    return -1;
}

int esp32_camera_gc032a_detect(int slv_addr, sensor_id_t *id)
{
    if (GC032A_SCCB_ADDR == slv_addr) {
        uint8_t MIDL = SCCB_Read(slv_addr, SENSOR_ID_LOW);
        uint8_t MIDH = SCCB_Read(slv_addr, SENSOR_ID_HIGH);
        uint16_t PID = MIDH << 8 | MIDL;
        if (GC032A_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int esp32_camera_gc032a_init(sensor_t *sensor)
{
    sensor->init_status = init_status;
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_contrast = set_dummy;
    sensor->set_brightness = set_dummy;
    sensor->set_saturation = set_dummy;
    sensor->set_sharpness = set_dummy;
    sensor->set_denoise = set_dummy;
    sensor->set_gainceiling = set_gainceiling_dummy;
    sensor->set_quality = set_dummy;
    sensor->set_colorbar = set_colorbar;
    sensor->set_whitebal = set_dummy;
    sensor->set_gain_ctrl = set_dummy;
    sensor->set_exposure_ctrl = set_dummy;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    sensor->set_aec2 = set_dummy;
    sensor->set_awb_gain = set_dummy;
    sensor->set_agc_gain = set_dummy;
    sensor->set_aec_value = set_dummy;

    sensor->set_special_effect = set_dummy;
    sensor->set_wb_mode = set_dummy;
    sensor->set_ae_level = set_dummy;

    sensor->set_dcw = set_dummy;
    sensor->set_bpc = set_dummy;
    sensor->set_wpc = set_dummy;

    sensor->set_raw_gma = set_dummy;
    sensor->set_lenc = set_dummy;

    sensor->get_reg = get_reg;
    sensor->set_reg = set_reg;
    sensor->set_res_raw = NULL;
    sensor->set_pll = NULL;
    sensor->set_xclk = NULL;

    ESP_LOGD(TAG, "GC032A Attached");
    return 0;
}
