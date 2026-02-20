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
    ESP_ERROR_CHECK(bsp_display_brightness_init());
    ESP_ERROR_CHECK(bsp_display_backlight_off());

    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    const bsp_display_config_t lcd_cfg = { .dummy = NULL };
    ESP_ERROR_CHECK(bsp_display_new(&lcd_cfg, &panel, &io));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    ESP_ERROR_CHECK(bsp_display_backlight_on());

    uint16_t *fb = heap_caps_malloc(BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    assert(fb);
    memset(fb, 0x1F, BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t));
    esp_lcd_panel_draw_bitmap(panel, 0, 0, BSP_LCD_H_RES, BSP_LCD_V_RES, fb);
    heap_caps_free(fb);

    /* Poll touch without LVGL */
    esp_lcd_touch_handle_t tp;
    const bsp_touch_config_t tp_cfg = { .dummy = NULL };
    ESP_ERROR_CHECK(bsp_touch_new(&tp_cfg, &tp));
    while (1) {
        esp_lcd_touch_read_data(tp);
        uint16_t x, y, strength;
        uint8_t count = 0;
        if (esp_lcd_touch_get_coordinates(tp, &x, &y, &strength, &count, 1) && count) {
            ESP_LOGI(TAG, "x=%d y=%d", x, y);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
