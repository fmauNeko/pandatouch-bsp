/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "bsp/pandatouch.h"
#include "bsp/display.h"
#include "bsp_err_check.h"
#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#include "esp_lvgl_port.h"

/* Forward declarations — implemented in bsp_touch.c */
esp_err_t bsp_display_indev_init(lv_display_t *disp);
void bsp_display_set_touch_indev(lv_indev_t *indev);
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0

static const char *TAG = "pandatouch";

esp_err_t bsp_display_brightness_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_1,
        .duty_resolution  = LEDC_TIMER_11_BIT,
        .freq_hz          = 30000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    BSP_ERROR_CHECK_RETURN_ERR(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BSP_LCD_BACKLIGHT,
        .duty           = 0,
        .hpoint         = 0,
    };
    BSP_ERROR_CHECK_RETURN_ERR(ledc_channel_config(&ledc_channel));

    return ESP_OK;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    uint32_t duty = (brightness_percent * ((1 << LEDC_TIMER_11_BIT) - 1)) / 100;
    BSP_ERROR_CHECK_RETURN_ERR(ledc_set_duty(LEDC_LOW_SPEED_MODE, CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH, duty));
    BSP_ERROR_CHECK_RETURN_ERR(ledc_update_duty(LEDC_LOW_SPEED_MODE, CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH));

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
    if (!ret_panel) {
        return ESP_ERR_INVALID_ARG;
    }
    if (ret_io) {
        *ret_io = NULL;
    }

    BSP_ERROR_CHECK_RETURN_ERR(bsp_display_brightness_init());

    /* LCD reset pulse */
    gpio_config_t io_conf = {
        .pin_bit_mask = BIT64(BSP_LCD_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    BSP_ERROR_CHECK_RETURN_ERR(gpio_config(&io_conf));
    BSP_ERROR_CHECK_RETURN_ERR(gpio_set_level(BSP_LCD_RST, 0));
    vTaskDelay(pdMS_TO_TICKS(100));
    BSP_ERROR_CHECK_RETURN_ERR(gpio_set_level(BSP_LCD_RST, 1));
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Configure RGB panel */
    esp_lcd_rgb_panel_config_t panel_conf = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz           = BSP_LCD_PIXEL_CLOCK_HZ,
            .h_res             = BSP_LCD_H_RES,
            .v_res             = BSP_LCD_V_RES,
            .hsync_pulse_width = 4,
            .hsync_back_porch  = 8,
            .hsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .vsync_back_porch  = 16,
            .vsync_front_porch = 16,
            .flags.pclk_active_neg = true,
        },
        .data_width             = 16,
        .in_color_format        = LCD_COLOR_FMT_RGB565,
        .num_fbs                = 2,  /* double-buffer required for avoid_tearing */
        .bounce_buffer_size_px  = BSP_LCD_H_RES * CONFIG_BSP_LCD_RGB_BOUNCE_BUF_HEIGHT,
        .hsync_gpio_num    = GPIO_NUM_NC,
        .vsync_gpio_num    = GPIO_NUM_NC,
        .de_gpio_num       = BSP_LCD_DE,
        .pclk_gpio_num     = BSP_LCD_PCLK,
        .data_gpio_nums    = {
            BSP_LCD_DATA0,   /* B3 */
            BSP_LCD_DATA1,   /* B4 */
            BSP_LCD_DATA2,   /* B5 */
            BSP_LCD_DATA3,   /* B6 */
            BSP_LCD_DATA4,   /* B7 */
            BSP_LCD_DATA5,   /* G2 */
            BSP_LCD_DATA6,   /* G3 */
            BSP_LCD_DATA7,   /* G4 */
            BSP_LCD_DATA8,   /* G5 */
            BSP_LCD_DATA9,   /* G6 */
            BSP_LCD_DATA10,  /* G7 */
            BSP_LCD_DATA11,  /* R3 */
            BSP_LCD_DATA12,  /* R4 */
            BSP_LCD_DATA13,  /* R5 */
            BSP_LCD_DATA14,  /* R6 */
            BSP_LCD_DATA15,  /* R7 */
        },
        .disp_gpio_num     = GPIO_NUM_NC,
        .flags.fb_in_psram = true,
    };
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_new_rgb_panel(&panel_conf, ret_panel));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_reset(*ret_panel));
    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_init(*ret_panel));

    return ESP_OK;
}

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static esp_lcd_panel_handle_t s_panel_handle    = NULL;
static lv_display_t          *s_display         = NULL;
static lv_indev_t            *s_touch_indev     = NULL;
static bool                   s_display_sleeping = false;

lv_display_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size   = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = CONFIG_BSP_LCD_DRAW_BUF_DOUBLE,
        .flags = {
            .buff_dma    = false,
            .buff_spiram = false,
        },
    };

    return bsp_display_start_with_config(&cfg);
}

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    BSP_NULL_CHECK(cfg, NULL);

    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&cfg->lvgl_port_cfg));

    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(NULL, &s_panel_handle, NULL));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle      = NULL,
        .panel_handle   = s_panel_handle,
        .control_handle = NULL,
        .buffer_size    = cfg->buffer_size,
        .double_buffer  = cfg->double_buffer,
        .hres           = BSP_LCD_H_RES,
        .vres           = BSP_LCD_V_RES,
        .monochrome     = false,
        .rotation       = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .flags = {
            .buff_dma     = cfg->flags.buff_dma,
            .buff_spiram  = cfg->flags.buff_spiram,
            .sw_rotate    = false,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes   = (BSP_LCD_BIGENDIAN ? true : false),
#endif
            .full_refresh = false,
            .direct_mode  = true,
        },
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode       = true,
            .avoid_tearing = true,
        },
    };

    s_display = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);

    if (!s_display) {
        ESP_LOGW(TAG, "Display creation failed — skipping touch init");
        return NULL;
    }

    /* Initialize touch input device */
    if (bsp_display_indev_init(s_display) != ESP_OK) {
        ESP_LOGW(TAG, "Touch indev init failed — continuing without touch");
    }

    return s_display;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return s_touch_indev;
}

void bsp_display_set_touch_indev(lv_indev_t *indev)
{
    s_touch_indev = indev;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation)
{
#if (LVGL_VERSION_MAJOR >= 9)
    lv_display_set_rotation(disp, (lv_display_rotation_t)rotation);
#else
    lv_disp_set_rotation(disp, rotation);
#endif
}

esp_err_t bsp_display_enter_sleep(void)
{
    if (!s_panel_handle || !s_display) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!bsp_display_lock(0)) {
        return ESP_ERR_TIMEOUT;
    }

    if (s_display_sleeping) {
        bsp_display_unlock();
        return ESP_OK;
    }

    lv_timer_t *refr_timer = lv_display_get_refr_timer(s_display);
    if (refr_timer) {
        lv_timer_pause(refr_timer);
    }

    void *fb0 = NULL;
    void *fb1 = NULL;
    esp_err_t fb_err = esp_lcd_rgb_panel_get_frame_buffer(s_panel_handle, 2, &fb0, &fb1);
    if (fb_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get frame buffer: %s", esp_err_to_name(fb_err));
    }
    if (fb0) {
        memset(fb0, 0, BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t));
    }
    if (fb1) {
        memset(fb1, 0, BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t));
    }

    s_display_sleeping = true;

    bsp_display_unlock();

    BSP_ERROR_CHECK_RETURN_ERR(bsp_display_backlight_off());

    return ESP_OK;
}

esp_err_t bsp_display_exit_sleep(void)
{
    if (!s_display) {
        return ESP_ERR_INVALID_STATE;
    }

    BSP_ERROR_CHECK_RETURN_ERR(bsp_display_backlight_on());

    if (!bsp_display_lock(0)) {
        return ESP_ERR_TIMEOUT;
    }

    if (!s_display_sleeping) {
        bsp_display_unlock();
        return ESP_OK;
    }

    lv_timer_t *refr_timer = lv_display_get_refr_timer(s_display);
    if (refr_timer) {
        lv_timer_resume(refr_timer);
    }

    lv_obj_invalidate(lv_display_get_screen_active(s_display));

    s_display_sleeping = false;

    bsp_display_unlock();

    return ESP_OK;
}
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0
