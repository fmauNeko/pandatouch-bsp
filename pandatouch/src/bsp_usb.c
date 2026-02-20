/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include "esp_err.h"
#include "bsp/pandatouch.h"

static bsp_usb_event_cb_t s_on_mount = NULL;
static bsp_usb_event_cb_t s_on_unmount = NULL;
static volatile bool s_mounted = false;

esp_err_t bsp_usb_start(void)
{
    return ESP_OK;
}

void bsp_usb_stop(void)
{
}

bool bsp_usb_is_mounted(void)
{
    return s_mounted;
}

void bsp_usb_on_mount(bsp_usb_event_cb_t cb)
{
    s_on_mount = cb;
}

void bsp_usb_on_unmount(bsp_usb_event_cb_t cb)
{
    s_on_unmount = cb;
}
