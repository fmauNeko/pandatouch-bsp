/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file main.c
 * @brief display_demo — comprehensive BSP showcase for the Panda Touch
 *
 * Three-tab LVGL UI:
 *  - Backlight  : Interactive slider to control PWM backlight brightness
 *  - USB        : File browser — lists files/directories from an inserted USB drive
 *  - Sensor     : Live temperature & humidity from the optional Panda Sense (AHT30)
 *                 Gracefully shows "not connected" when the module is absent
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "bsp/esp-bsp.h"
#include "aht30.h"

static const char *TAG = "demo";

/* ── AHT30 on external I2C header (I2C1 / GPIO3+4) ─────────────────────── */
static i2c_master_bus_handle_t s_ext_i2c  = NULL;
static aht30_handle_t          s_aht30    = NULL;
static bool                    s_sensor_ok = false;

/* ── LVGL widget refs (set during ui_create, used in callbacks) ─────────── */
static lv_obj_t *s_brightness_label = NULL;
static lv_obj_t *s_usb_list         = NULL;
static lv_obj_t *s_usb_status       = NULL;
static lv_obj_t *s_temp_label       = NULL;
static lv_obj_t *s_hum_label        = NULL;

/* ── Colour palette ─────────────────────────────────────────────────────── */
#define COL_BG        lv_color_hex(0x1a1a2e)
#define COL_CARD      lv_color_hex(0x16213e)
#define COL_ACCENT    lv_color_hex(0xe94560)
#define COL_CYAN      lv_color_hex(0x4cc9f0)
#define COL_BORDER    lv_color_hex(0x0f3460)
#define COL_TEXT      lv_color_hex(0xeeeeee)
#define COL_MUTED     lv_color_hex(0x888888)

/* ════════════════════════════════════════════════════════════════════════════
 *  AHT30 initialisation
 *  Uses I2C1 (BSP_EXT_I2C_NUM) on the external 4-pin I2C header.
 *  Called before bsp_display_start() — no LVGL lock needed.
 * ════════════════════════════════════════════════════════════════════════════ */
static void sensor_init(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port                     = BSP_EXT_I2C_NUM,
        .sda_io_num                   = BSP_EXT_I2C_SDA,
        .scl_io_num                   = BSP_EXT_I2C_SCL,
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = true,
    };

    if (i2c_new_master_bus(&bus_cfg, &s_ext_i2c) != ESP_OK) {
        ESP_LOGW(TAG, "Ext I2C bus init failed — AHT30 unavailable");
        return;
    }

    if (aht30_create(s_ext_i2c, AHT30_I2C_ADDRESS, &s_aht30) != ESP_OK) {
        ESP_LOGW(TAG, "AHT30 not found on I2C1 — sensor tab will show N/A");
        i2c_del_master_bus(s_ext_i2c);
        s_ext_i2c = NULL;
        return;
    }

    s_sensor_ok = true;
    ESP_LOGI(TAG, "AHT30 ready on I2C1 (GPIO%d/GPIO%d)",
             BSP_EXT_I2C_SCL, BSP_EXT_I2C_SDA);
}

/* ════════════════════════════════════════════════════════════════════════════
 *  USB file list
 *  MUST be called with the LVGL lock held.
 * ════════════════════════════════════════════════════════════════════════════ */
static void usb_list_refresh(void)
{
    lv_obj_clean(s_usb_list);

    if (!bsp_usb_is_mounted()) {
        lv_label_set_text(s_usb_status, LV_SYMBOL_USB "  No USB drive connected");
        lv_obj_clear_flag(s_usb_status, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_usb_list, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    DIR *dir = opendir("/usb");
    if (!dir) {
        lv_label_set_text(s_usb_status, LV_SYMBOL_WARNING "  Failed to open /usb");
        lv_obj_clear_flag(s_usb_status, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_usb_list, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] == '.') {
            continue;  /* skip . and .. */
        }
        const char *icon = (ent->d_type == DT_DIR)
                           ? LV_SYMBOL_DIRECTORY "  "
                           : LV_SYMBOL_FILE      "  ";
        char line[272];  /* LV_SYMBOL (6 bytes) + "  " (2) + NAME_MAX (255) + NUL */
        snprintf(line, sizeof(line), "%s%s", icon, ent->d_name);
        lv_list_add_text(s_usb_list, line);
        count++;
    }
    closedir(dir);

    if (count == 0) {
        lv_obj_add_flag(s_usb_list, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(s_usb_status, LV_SYMBOL_USB "  Drive is empty");
        lv_obj_clear_flag(s_usb_status, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(s_usb_status, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_usb_list, LV_OBJ_FLAG_HIDDEN);
    }
}

/* USB mount / unmount callbacks — run in the MSC task, not the LVGL task */
static void on_usb_mount(void)
{
    ESP_LOGI(TAG, "USB mounted");
    if (bsp_display_lock(500)) {
        usb_list_refresh();
        bsp_display_unlock();
    }
}

static void on_usb_unmount(void)
{
    ESP_LOGI(TAG, "USB removed");
    if (bsp_display_lock(500)) {
        usb_list_refresh();
        bsp_display_unlock();
    }
}

/* ════════════════════════════════════════════════════════════════════════════
 *  LVGL event / timer callbacks
 *  All run inside the LVGL task — no extra lock needed.
 * ════════════════════════════════════════════════════════════════════════════ */
static void brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int val = (int)lv_slider_get_value(slider);
    bsp_display_brightness_set(val);
    lv_label_set_text_fmt(s_brightness_label, "%d%%", val);
}

static void sensor_timer_cb(lv_timer_t *t)
{
    (void)t;
    float temp = 0.0f, hum = 0.0f;
    if (aht30_get_temperature_humidity_value(s_aht30, &temp, &hum) == ESP_OK) {
        lv_label_set_text_fmt(s_temp_label, LV_SYMBOL_CHARGE "  %.1f " "\xc2\xb0" "C",  temp);
        lv_label_set_text_fmt(s_hum_label,  LV_SYMBOL_REFRESH " %.1f %%RH", hum);
    } else {
        lv_label_set_text(s_temp_label, LV_SYMBOL_WARNING "  Read error");
        lv_label_set_text(s_hum_label,  "");
    }
}

/* ════════════════════════════════════════════════════════════════════════════
 *  UI builder
 *  Must be called inside bsp_display_lock() / bsp_display_unlock().
 * ════════════════════════════════════════════════════════════════════════════ */
static void ui_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ── Tab view ─────────────────────────────────────────────────────────── */
    lv_obj_t *tv = lv_tabview_create(scr);
    lv_tabview_set_tab_bar_position(tv, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tv, 52);
    lv_obj_set_size(tv, BSP_LCD_H_RES, BSP_LCD_V_RES);
    lv_obj_set_style_bg_color(tv, COL_BG, 0);

    /* Tab bar */
    lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
    lv_obj_set_style_bg_color(tab_bar, COL_BORDER, 0);
    lv_obj_set_style_bg_opa(tab_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(tab_bar, COL_TEXT, 0);
    lv_obj_set_style_text_font(tab_bar, &lv_font_montserrat_18, 0);

    lv_obj_t *tab_bl  = lv_tabview_add_tab(tv, LV_SYMBOL_IMAGE  " Backlight");
    lv_obj_t *tab_usb = lv_tabview_add_tab(tv, LV_SYMBOL_USB    " USB");
    lv_obj_t *tab_sen = lv_tabview_add_tab(tv, LV_SYMBOL_CHARGE " Sensor");

    /* Common tab content style */
    lv_obj_t *tabs[] = {tab_bl, tab_usb, tab_sen};
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_bg_color(tabs[i], COL_BG, 0);
        lv_obj_set_style_bg_opa(tabs[i], LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(tabs[i], 20, 0);
    }

    /* ── Tab 1: Backlight ─────────────────────────────────────────────────── */
    lv_obj_set_flex_flow(tab_bl, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab_bl, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tab_bl, 28, 0);

    lv_obj_t *bl_title = lv_label_create(tab_bl);
    lv_label_set_text(bl_title, "Screen Brightness");
    lv_obj_set_style_text_font(bl_title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(bl_title, COL_MUTED, 0);

    lv_obj_t *slider = lv_slider_create(tab_bl);
    lv_slider_set_range(slider, 1, 100);
    lv_slider_set_value(slider, 80, LV_ANIM_OFF);
    lv_obj_set_width(slider, 640);
    lv_obj_set_height(slider, 28);
    lv_obj_set_style_bg_color(slider, COL_CARD,   LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, COL_BORDER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, COL_ACCENT, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 8, LV_PART_KNOB);
    lv_obj_add_event_cb(slider, brightness_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_brightness_label = lv_label_create(tab_bl);
    lv_label_set_text(s_brightness_label, "80%");
    lv_obj_set_style_text_font(s_brightness_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_brightness_label, COL_ACCENT, 0);

    /* ── Tab 2: USB ───────────────────────────────────────────────────────── */
    lv_obj_set_flex_flow(tab_usb, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab_usb, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tab_usb, 10, 0);

    lv_obj_t *usb_title = lv_label_create(tab_usb);
    lv_label_set_text(usb_title, "USB Drive Contents");
    lv_obj_set_style_text_font(usb_title, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(usb_title, COL_MUTED, 0);

    s_usb_status = lv_label_create(tab_usb);
    lv_label_set_text(s_usb_status, LV_SYMBOL_USB "  No USB drive connected");
    lv_obj_set_style_text_color(s_usb_status, COL_MUTED, 0);
    lv_obj_set_style_text_font(s_usb_status, &lv_font_montserrat_16, 0);

    s_usb_list = lv_list_create(tab_usb);
    lv_obj_set_width(s_usb_list, lv_pct(100));
    lv_obj_set_flex_grow(s_usb_list, 1);
    lv_obj_set_style_bg_color(s_usb_list, COL_CARD, 0);
    lv_obj_set_style_bg_opa(s_usb_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_usb_list, COL_BORDER, 0);
    lv_obj_set_style_border_width(s_usb_list, 1, 0);
    lv_obj_set_style_text_color(s_usb_list, COL_TEXT, 0);
    lv_obj_set_style_text_font(s_usb_list, &lv_font_montserrat_16, 0);
    lv_obj_add_flag(s_usb_list, LV_OBJ_FLAG_HIDDEN);

    /* ── Tab 3: Sensor ────────────────────────────────────────────────────── */
    lv_obj_set_flex_flow(tab_sen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tab_sen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(tab_sen, 28, 0);

    if (s_sensor_ok) {
        lv_obj_t *sen_title = lv_label_create(tab_sen);
        lv_label_set_text(sen_title, "Panda Sense  \xe2\x80\x94  AHT30");
        lv_obj_set_style_text_font(sen_title, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(sen_title, COL_MUTED, 0);

        s_temp_label = lv_label_create(tab_sen);
        lv_label_set_text(s_temp_label, LV_SYMBOL_CHARGE "  ---.-" "\xc2\xb0" "C");
        lv_obj_set_style_text_font(s_temp_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(s_temp_label, COL_ACCENT, 0);

        s_hum_label = lv_label_create(tab_sen);
        lv_label_set_text(s_hum_label, LV_SYMBOL_REFRESH "  ---.-%RH");
        lv_obj_set_style_text_font(s_hum_label, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(s_hum_label, COL_CYAN, 0);

        /* Fire immediately on the first LVGL tick, then every 2 s */
        lv_timer_t *t = lv_timer_create(sensor_timer_cb, 2000, NULL);
        lv_timer_ready(t);
    } else {
        lv_obj_t *msg = lv_label_create(tab_sen);
        lv_label_set_text(msg,
            LV_SYMBOL_WARNING "  Sensor not connected\n\n"
            "Attach the Panda Sense module\n"
            "to the I2C1 header (GPIO 3 / 4)");
        lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(msg, 560);
        lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(msg, COL_MUTED, 0);
        lv_obj_set_style_text_font(msg, &lv_font_montserrat_18, 0);
    }
}

/* ════════════════════════════════════════════════════════════════════════════
 *  Entry point
 * ════════════════════════════════════════════════════════════════════════════ */
void app_main(void)
{
    /* 1. Probe AHT30 before LVGL starts — I2C bus init has no LVGL dependency */
    sensor_init();

    /* 2. Start display + touch + LVGL */
    lv_display_t *disp = bsp_display_start();
    assert(disp);
    bsp_display_brightness_set(80);

    /* 3. Build UI (must hold LVGL mutex) */
    bsp_display_lock(0);
    ui_create();
    bsp_display_unlock();

    /* 4. Register USB callbacks and start MSC host */
    bsp_usb_on_mount(on_usb_mount);
    bsp_usb_on_unmount(on_usb_unmount);
    bsp_usb_start();
}
