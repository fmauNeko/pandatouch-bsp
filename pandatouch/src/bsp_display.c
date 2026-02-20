/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#include "bsp/pandatouch.h"
#include "bsp/display.h"
#include "bsp_err_check.h"

static const char *TAG = "pandatouch";

esp_err_t bsp_display_brightness_init(void)
{
    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    return ESP_OK;
}

esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_new(const bsp_display_config_t *config,
                           esp_lcd_panel_handle_t     *ret_panel,
                           esp_lcd_panel_io_handle_t  *ret_io)
{
    if (ret_io) {
        *ret_io = NULL;
    }
    return ESP_OK;
}

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
lv_display_t *bsp_display_start(void)
{
    return NULL;
}

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    return NULL;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return NULL;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return true;
}

void bsp_display_unlock(void)
{
}

void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation)
{
}

esp_err_t bsp_display_enter_sleep(void)
{
    return ESP_OK;
}

esp_err_t bsp_display_exit_sleep(void)
{
    return ESP_OK;
}
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0
