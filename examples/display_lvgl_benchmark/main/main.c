/**
 * @file main.c
 * @brief LVGL Benchmark
 * @details Runs the built-in LVGL benchmark suite and prints a performance summary to the serial console.
 */

#include "lv_demos.h"
#include "bsp/esp-bsp.h"

#include "esp_log.h"

static const char *TAG = "benchmark";

void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    assert(disp);

    ESP_ERROR_CHECK(bsp_display_backlight_on());

    ESP_LOGI(TAG, "Running LVGL benchmark");

    bsp_display_lock(0);
    lv_demo_benchmark();
    bsp_display_unlock();
}
