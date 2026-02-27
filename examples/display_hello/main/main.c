/**
 * @file main.c
 * @brief Hello Panda Touch
 * @details Simple "Hello World" example showing a centered label and USB MSC host initialization.
 */

#include "bsp/esp-bsp.h"

#include "esp_log.h"

static void on_usb_mount(void) { ESP_LOGI("app", "USB mounted"); }

void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    assert(disp);
    bsp_display_brightness_set(80);

    if (!bsp_display_lock(portMAX_DELAY)) {
        ESP_LOGE("app", "Failed to acquire display lock");
        return;
    }
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, Panda Touch!");
    lv_obj_center(label);
    bsp_display_unlock();

    bsp_usb_on_mount(on_usb_mount);
    ESP_ERROR_CHECK(bsp_usb_start());
}
