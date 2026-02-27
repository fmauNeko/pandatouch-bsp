/**
 * @file main.c
 * @brief LVGL Benchmark
 * @details Runs the built-in LVGL benchmark suite and prints a performance summary to the serial console.
 */

#include "lv_demos.h"
#include "bsp/esp-bsp.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "benchmark";

void lv_mem_init(void)   { }
void lv_mem_deinit(void) { }

void *lv_malloc_core(size_t size)
{
    void *p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) {
        p = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return p;
}

void *lv_realloc_core(void *p, size_t new_size)
{
    if (new_size == 0) {
        heap_caps_free(p);
        return NULL;
    }
    void *new_p = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!new_p) {
        new_p = heap_caps_realloc(p, new_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    return new_p;
}

void lv_free_core(void *p)
{
    heap_caps_free(p);
}

void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    assert(disp);

    ESP_ERROR_CHECK(bsp_display_backlight_on());

    ESP_LOGI(TAG, "Running LVGL benchmark");

    if (bsp_display_lock(0)) {
        lv_demo_benchmark();
        bsp_display_unlock();
    } else {
        ESP_LOGE(TAG, "Failed to acquire display lock");
    }
}
