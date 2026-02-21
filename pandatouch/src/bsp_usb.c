/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "usb/usb_host.h"
#include "usb/msc_host_vfs.h"
#include "bsp/pandatouch.h"
#include "bsp_err_check.h"

static const char *TAG = "bsp_usb";

/*
 * Internal event types for decoupling the MSC callback from install/mount operations.
 *
 * msc_event_cb() runs in the MSC driver's background task.  Calling
 * msc_host_install_device() from inside that callback would deadlock because
 * the driver itself is waiting for the install to complete.  We therefore
 * enqueue a lightweight message and let msc_app_task() do the heavy lifting.
 */
typedef enum {
    USB_MSC_EVT_CONNECTED,
    USB_MSC_EVT_DISCONNECTED,
} usb_msc_evt_type_t;

typedef struct {
    usb_msc_evt_type_t type;
    union {
        uint8_t                  address; /*!< USB device address (CONNECTED event)    */
        msc_host_device_handle_t handle;  /*!< Device handle     (DISCONNECTED event)  */
    };
} usb_msc_evt_t;

/* Registered callbacks */
static bsp_usb_event_cb_t s_on_mount   = NULL;
static bsp_usb_event_cb_t s_on_unmount = NULL;

/* State */
static volatile bool             s_mounted         = false;
static msc_host_device_handle_t  s_msc_device      = NULL;
static msc_host_vfs_handle_t     s_vfs_handle      = NULL;
static TaskHandle_t              s_usb_host_task   = NULL;
static TaskHandle_t              s_msc_app_task    = NULL;
static QueueHandle_t             s_usb_event_queue = NULL;

/* -------------------------------------------------------------------------
 * USB Host Library event task
 * Handles low-level USB host library events (client/device lifecycle).
 * -------------------------------------------------------------------------*/
static void usb_host_task(void *arg)
{
    while (1) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            usb_host_device_free_all();
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            break;
        }
    }
    vTaskDelete(NULL);
}

/* -------------------------------------------------------------------------
 * MSC event callback
 * Runs inside the MSC driver's own background task.
 * IMPORTANT: Do NOT call msc_host_install_device() or msc_host_vfs_register()
 * here — forward events to msc_app_task() via the queue instead.
 * -------------------------------------------------------------------------*/
static void msc_event_cb(const msc_host_event_t *event, void *arg)
{
    usb_msc_evt_t evt;

    if (event->event == MSC_DEVICE_CONNECTED) {
        evt.type    = USB_MSC_EVT_CONNECTED;
        evt.address = event->device.address;
        xQueueSend(s_usb_event_queue, &evt, 0);
    } else if (event->event == MSC_DEVICE_DISCONNECTED) {
        evt.type   = USB_MSC_EVT_DISCONNECTED;
        evt.handle = event->device.handle;
        xQueueSend(s_usb_event_queue, &evt, 0);
    }
}

static const msc_host_driver_config_t s_msc_driver_config = {
    .create_backround_task = true,
    .task_priority         = 5,
    .stack_size            = 4096,
    .callback              = msc_event_cb,
};

/* -------------------------------------------------------------------------
 * MSC application task
 * Installs the MSC device and mounts the VFS on connect;
 * unmounts and uninstalls on disconnect.
 * -------------------------------------------------------------------------*/
static void msc_app_task(void *arg)
{
    usb_msc_evt_t evt;

    while (xQueueReceive(s_usb_event_queue, &evt, portMAX_DELAY)) {

        if (evt.type == USB_MSC_EVT_CONNECTED) {
            ESP_LOGI(TAG, "MSC device connected");

            esp_err_t ret = msc_host_install_device(evt.address, &s_msc_device);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "msc_host_install_device failed: %s", esp_err_to_name(ret));
                s_msc_device = NULL;
                continue;
            }

            const esp_vfs_fat_mount_config_t mount_config = {
                .format_if_mount_failed = false,
                .max_files              = 3,
                .allocation_unit_size   = 8192,
            };
            ret = msc_host_vfs_register(s_msc_device, "/usb", &mount_config, &s_vfs_handle);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "msc_host_vfs_register failed: %s", esp_err_to_name(ret));
                msc_host_uninstall_device(s_msc_device);
                s_msc_device = NULL;
                s_vfs_handle = NULL;
                continue;
            }

            s_mounted = true;
            ESP_LOGI(TAG, "USB MSC mounted at /usb");
            if (s_on_mount) {
                s_on_mount();
            }

        } else if (evt.type == USB_MSC_EVT_DISCONNECTED) {
            ESP_LOGI(TAG, "MSC device disconnected");

            s_mounted = false;
            if (s_on_unmount) {
                s_on_unmount();
            }

            if (s_vfs_handle) {
                msc_host_vfs_unregister(s_vfs_handle);
                s_vfs_handle = NULL;
            }
            if (s_msc_device) {
                msc_host_uninstall_device(s_msc_device);
                s_msc_device = NULL;
            }

            ESP_LOGI(TAG, "Reinitialising MSC driver for next hotplug");
            esp_err_t reinit_ret = msc_host_uninstall();
            if (reinit_ret != ESP_OK) {
                ESP_LOGE(TAG, "msc_host_uninstall failed: %s", esp_err_to_name(reinit_ret));
            } else {
                reinit_ret = msc_host_install(&s_msc_driver_config);
                if (reinit_ret != ESP_OK) {
                    ESP_LOGE(TAG, "msc_host_install (reinit) failed: %s", esp_err_to_name(reinit_ret));
                } else {
                    ESP_LOGI(TAG, "MSC driver reinitialised, ready for next plug-in");
                }
            }
        }
    }

    vTaskDelete(NULL);
}

/* =========================================================================
 * Public API
 * =========================================================================*/

esp_err_t bsp_usb_start(void)
{
    s_usb_event_queue = xQueueCreate(5, sizeof(usb_msc_evt_t));
    if (!s_usb_event_queue) {
        return ESP_ERR_NO_MEM;
    }

    /* 1. Install USB host stack */
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags     = ESP_INTR_FLAG_LEVEL1,
    };
    BSP_ERROR_CHECK_RETURN_ERR(usb_host_install(&host_config));

    /* 2. Start USB host library event task */
    if (xTaskCreate(usb_host_task, "usb_host", 4096, NULL, 5, &s_usb_host_task) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to create USB host task");
        return ESP_FAIL;
    }

    /* 3. Install MSC host driver (creates its own background task) */
    BSP_ERROR_CHECK_RETURN_ERR(msc_host_install(&s_msc_driver_config));

    /* 4. Start app task that processes device connect/disconnect events */
    if (xTaskCreate(msc_app_task, "msc_app", 4096, NULL, 5, &s_msc_app_task) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to create MSC app task");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void bsp_usb_stop(void)
{
    /* Unmount and release device handles before tearing down the drivers */
    if (s_vfs_handle) {
        msc_host_vfs_unregister(s_vfs_handle);
        s_vfs_handle = NULL;
    }
    if (s_msc_device) {
        msc_host_uninstall_device(s_msc_device);
        s_msc_device = NULL;
    }

    /* Deregister MSC as a USB client (stops MSC background task) */
    msc_host_uninstall();

    /* Free remaining USB devices, then uninstall the host stack.
     * usb_host_task() will detect USB_HOST_LIB_EVENT_FLAGS_ALL_FREE and exit. */
    usb_host_device_free_all();
    usb_host_uninstall();

    s_mounted = false;

    /* Stop the application task and release the event queue */
    if (s_msc_app_task) {
        vTaskDelete(s_msc_app_task);
        s_msc_app_task = NULL;
    }
    if (s_usb_event_queue) {
        vQueueDelete(s_usb_event_queue);
        s_usb_event_queue = NULL;
    }

    s_usb_host_task = NULL; /* task exits on its own after ALL_FREE */
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
