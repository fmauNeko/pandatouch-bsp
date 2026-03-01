/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file main.cpp
 * @brief display_slint — Panda Touch Slint UI example
 * @details Three-tab Slint UI demonstrating backlight control, USB file browser, and optional AHT30 sensor data.
 */

#include <slint-esp.h>
#include <slint.h>
#include <span>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include <dirent.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_rgb.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "aht30.h"

#include "ui.h"

static const char *TAG = "display_slint";

static i2c_master_bus_handle_t s_ext_i2c = nullptr;
static aht30_handle_t s_aht30 = nullptr;
static bool s_sensor_ok = false;

static void sensor_init(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = BSP_EXT_I2C_NUM,
        .sda_io_num = BSP_EXT_I2C_SDA,
        .scl_io_num = BSP_EXT_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = { .enable_internal_pullup = true, .allow_pd = false },
    };

    if (i2c_new_master_bus(&bus_cfg, &s_ext_i2c) != ESP_OK) {
        ESP_LOGW(TAG, "Ext I2C bus init failed — AHT30 unavailable");
        return;
    }

    if (aht30_create(s_ext_i2c, AHT30_I2C_ADDRESS, &s_aht30) != ESP_OK) {
        ESP_LOGW(TAG, "AHT30 not found on I2C1 — sensor tab will show N/A");
        i2c_del_master_bus(s_ext_i2c);
        s_ext_i2c = nullptr;
        return;
    }

    s_sensor_ok = true;
}

static void sensor_task(void *arg)
{
    auto *weak_ui_ptr = static_cast<slint::ComponentWeakHandle<AppWindow> *>(arg);
    while (true) {
        float temp = 0.0f;
        float hum = 0.0f;
        bool ok = (aht30_get_temperature_humidity_value(s_aht30, &temp, &hum) == ESP_OK);

        char temp_buf[32] = {0};
        char hum_buf[32] = {0};
        if (ok) {
            snprintf(temp_buf, sizeof(temp_buf), "%.1f °C", temp);
            snprintf(hum_buf, sizeof(hum_buf), "%.1f %%", hum);
        }

        slint::invoke_from_event_loop([
            weak_ui_ptr,
            ok,
            temp_str = std::string(ok ? temp_buf : ""),
            hum_str = std::string(ok ? hum_buf : "")
        ]() {
            if (auto handle = weak_ui_ptr->lock()) {
                auto &ui = *handle;
                ui->set_sensor_connected(ok);
                if (ok) {
                    ui->set_temperature(slint::SharedString(temp_str.c_str()));
                    ui->set_humidity(slint::SharedString(hum_str.c_str()));
                }
            }
        });

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static std::optional<slint::ComponentWeakHandle<AppWindow>> s_usb_ui;

static void usb_update(const slint::ComponentWeakHandle<AppWindow> &weak_ui)
{
    bool mounted = bsp_usb_is_mounted();
    std::vector<slint::SharedString> files;
    std::string status_str;

    if (!mounted) {
        status_str = "No USB Device";
    } else {
        DIR *dir = opendir("/usb");
        if (!dir) {
            status_str = "Failed to open /usb";
        } else {
            status_str = "USB Mounted";
            struct dirent *ent;
            while ((ent = readdir(dir)) != nullptr) {
                if (ent->d_name[0] == '.') {
                    continue;
                }
                std::string prefix = (ent->d_type == DT_DIR) ? "[D] " : "[F] ";
                files.push_back(slint::SharedString((prefix + ent->d_name).c_str()));
            }
            closedir(dir);
        }
    }

    slint::invoke_from_event_loop([
        weak_ui,
        files = std::move(files),
        status_str
    ]() mutable {
        if (auto handle = weak_ui.lock()) {
            auto &ui = *handle;
            auto model = std::make_shared<slint::VectorModel<slint::StandardListViewItem>>();
            for (auto &f : files) {
                model->push_back(slint::StandardListViewItem{f});
            }
            ui->set_usb_files(model);
            ui->set_usb_status(slint::SharedString(status_str.c_str()));
        }
    });
}

static void on_usb_mount(void)
{
    ESP_LOGI(TAG, "USB mounted");
    if (s_usb_ui) {
        usb_update(*s_usb_ui);
    }
}

static void on_usb_unmount(void)
{
    ESP_LOGI(TAG, "USB removed");
    if (s_usb_ui) {
        usb_update(*s_usb_ui);
    }
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init());

    esp_lcd_panel_handle_t panel_handle = nullptr;
    esp_lcd_panel_io_handle_t io_handle = nullptr;
    bsp_display_config_t disp_cfg = {};
    ESP_ERROR_CHECK(bsp_display_new(&disp_cfg, &panel_handle, &io_handle));

    ESP_ERROR_CHECK(bsp_display_brightness_init());
    ESP_ERROR_CHECK(bsp_display_backlight_on());

    esp_lcd_touch_handle_t touch_handle = nullptr;
    bsp_touch_config_t touch_cfg = {};
    ESP_ERROR_CHECK(bsp_touch_new(&touch_cfg, &touch_handle));

    void *buf1 = nullptr;
    void *buf2 = nullptr;
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));

    size_t fb_size = BSP_LCD_H_RES * BSP_LCD_V_RES;
    SlintPlatformConfiguration config{
        .size = slint::PhysicalSize({BSP_LCD_H_RES, BSP_LCD_V_RES}),
        .panel_handle = panel_handle,
        .touch_handle = touch_handle,
        .buffer1 = std::span<slint::platform::Rgb565Pixel>(
            static_cast<slint::platform::Rgb565Pixel *>(buf1), fb_size),
        .buffer2 = std::span<slint::platform::Rgb565Pixel>(
            static_cast<slint::platform::Rgb565Pixel *>(buf2), fb_size),
        .rotation = slint::platform::SoftwareRenderer::RenderingRotation::NoRotation,
        .byte_swap = false,
    };
    slint_esp_init(config);

    auto ui = AppWindow::create();
    ui->on_brightness_changed([](float value) {
        bsp_display_brightness_set(static_cast<int>(value));
    });
    ui->set_brightness(80);
    bsp_display_brightness_set(80);

    slint::ComponentWeakHandle<AppWindow> weak_ui(ui);
    s_usb_ui = weak_ui;
    bsp_usb_on_mount(on_usb_mount);
    bsp_usb_on_unmount(on_usb_unmount);
    if (bsp_usb_start() != ESP_OK) {
        ESP_LOGW(TAG, "USB MSC host init failed");
    }
    usb_update(weak_ui);

    sensor_init();
    slint::ComponentWeakHandle<AppWindow> *weak_ptr = nullptr;
    if (s_sensor_ok) {
        weak_ptr = new slint::ComponentWeakHandle<AppWindow>(weak_ui);
        xTaskCreatePinnedToCore(sensor_task, "sensor", 4096, weak_ptr, 4, nullptr, 1);
    } else {
        ui->set_sensor_connected(false);
    }

    ui->run();
}
