/**
 * @file main.c
 * @brief display_noglib — raw panel access example
 * @details Demonstrates direct framebuffer access without LVGL using the pandatouch_noglib component variant.
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp/touch.h"

static const char *TAG = "display_noglib";

void app_main(void)
{
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    const bsp_display_config_t lcd_cfg = { .dummy = NULL };
    ESP_ERROR_CHECK(bsp_display_new(&lcd_cfg, &panel, &io));
    ESP_ERROR_CHECK(bsp_display_backlight_on());

    uint16_t *fb = heap_caps_malloc(BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!fb) {
        ESP_LOGE(TAG, "framebuffer alloc failed");
        return;
    }
    memset(fb, 0x1F, BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t));
    esp_lcd_panel_draw_bitmap(panel, 0, 0, BSP_LCD_H_RES, BSP_LCD_V_RES, fb);
    heap_caps_free(fb);

    esp_lcd_touch_handle_t tp;
    const bsp_touch_config_t tp_cfg = { .dummy = NULL };
    ESP_ERROR_CHECK(bsp_touch_new(&tp_cfg, &tp));
    while (1) {
        esp_lcd_touch_read_data(tp);
        esp_lcd_touch_point_data_t pts[1];
        uint8_t count = 0;
        if (esp_lcd_touch_get_data(tp, pts, &count, 1) == ESP_OK && count) {
            ESP_LOGI(TAG, "x=%d y=%d", pts[0].x, pts[0].y);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
