/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#include "bsp/pandatouch.h"
#include "bsp/touch.h"
#include "bsp_err_check.h"

static i2c_master_bus_handle_t i2c_handle = NULL;
static bool i2c_initialized = false;

esp_err_t bsp_i2c_init(void)
{
    if (i2c_initialized) {
        return ESP_OK;
    }
    i2c_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    i2c_initialized = false;
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    bsp_i2c_init();
    return i2c_handle;
}

esp_err_t bsp_touch_new(const bsp_touch_config_t *config,
                         esp_lcd_touch_handle_t   *ret_touch)
{
    return ESP_OK;
}
