#include "bsp/esp-bsp.h"

static void on_usb_mount(void) { ESP_LOGI("app", "USB mounted"); }

void app_main(void)
{
    lv_display_t *disp = bsp_display_start();
    assert(disp);
    bsp_display_brightness_set(80);

    bsp_display_lock(0);
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, Panda Touch!");
    lv_obj_center(label);
    bsp_display_unlock();

    bsp_usb_on_mount(on_usb_mount);
    bsp_usb_start();
}
