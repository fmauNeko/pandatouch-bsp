/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * @brief ESP BSP: Panda Touch
 */

#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "bsp/config.h"
#include "bsp/display.h"
#include "bsp/touch.h"

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
#include "lvgl.h"
#include "esp_lvgl_port.h"
#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0

/**************************************************************************************************
 *  BSP Board Name
 **************************************************************************************************/

/** @defgroup boardname Board Name
 *  @brief BSP Board Name
 *  @{
 */
#define BSP_BOARD_PANDATOUCH
/** @} */

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

/** @defgroup capabilities Capabilities
 *  @brief BSP Capabilities
 *  @{
 */
#define BSP_CAPS_DISPLAY        1
#define BSP_CAPS_TOUCH          1
#define BSP_CAPS_BUTTONS        0
#define BSP_CAPS_AUDIO          0
#define BSP_CAPS_AUDIO_SPEAKER  0
#define BSP_CAPS_AUDIO_MIC      0
#define BSP_CAPS_SDCARD         0
#define BSP_CAPS_IMU            0
#define BSP_CAPS_USB_MSC        1
/** @} */

/**************************************************************************************************
 *  Panda Touch Pinout
 **************************************************************************************************/

/** @defgroup g01_i2c I2C
 *  @brief I2C BSP API
 *  @{
 */
#define BSP_I2C_NUM         CONFIG_BSP_I2C_NUM
#define BSP_I2C_SCL         (GPIO_NUM_1)
#define BSP_I2C_SDA         (GPIO_NUM_2)
/** @} */

/** @defgroup g04_display Display and Touch
 *  @brief Display BSP API
 *  @{
 */
/* RGB LCD */
#define BSP_LCD_PCLK        (GPIO_NUM_5)
#define BSP_LCD_DE          (GPIO_NUM_38)
#define BSP_LCD_RST         (GPIO_NUM_46)
#define BSP_LCD_BACKLIGHT   (GPIO_NUM_21)

/* Data bus D[0..15] = [B3..B7, G2..G7, R3..R7] */
#define BSP_LCD_DATA0       (GPIO_NUM_17)   /*!< B3 */
#define BSP_LCD_DATA1       (GPIO_NUM_18)   /*!< B4 */
#define BSP_LCD_DATA2       (GPIO_NUM_48)   /*!< B5 */
#define BSP_LCD_DATA3       (GPIO_NUM_47)   /*!< B6 */
#define BSP_LCD_DATA4       (GPIO_NUM_39)   /*!< B7 */
#define BSP_LCD_DATA5       (GPIO_NUM_11)   /*!< G2 */
#define BSP_LCD_DATA6       (GPIO_NUM_12)   /*!< G3 */
#define BSP_LCD_DATA7       (GPIO_NUM_13)   /*!< G4 */
#define BSP_LCD_DATA8       (GPIO_NUM_14)   /*!< G5 */
#define BSP_LCD_DATA9       (GPIO_NUM_15)   /*!< G6 */
#define BSP_LCD_DATA10      (GPIO_NUM_16)   /*!< G7 */
#define BSP_LCD_DATA11      (GPIO_NUM_6)    /*!< R3 */
#define BSP_LCD_DATA12      (GPIO_NUM_7)    /*!< R4 */
#define BSP_LCD_DATA13      (GPIO_NUM_8)    /*!< R5 */
#define BSP_LCD_DATA14      (GPIO_NUM_9)    /*!< R6 */
#define BSP_LCD_DATA15      (GPIO_NUM_10)   /*!< R7 */

/* Touch */
#define BSP_TOUCH_RST       (GPIO_NUM_41)
#define BSP_TOUCH_INT       (GPIO_NUM_40)
/** @} */

/** @defgroup g07_usb USB
 *  @brief USB BSP API
 *  @{
 */
#define BSP_USB_DP          (GPIO_NUM_20)
#define BSP_USB_DM          (GPIO_NUM_19)
/** @} */

/** @defgroup g08_ext_i2c Expansion I2C
 *  @brief Expansion I2C BSP API
 *  @{
 */
#define BSP_EXT_I2C_NUM     I2C_NUM_1
#define BSP_EXT_I2C_SCL     (GPIO_NUM_3)
#define BSP_EXT_I2C_SDA     (GPIO_NUM_4)
/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 *
 * I2C interface
 *
 * The GT911 touch controller is connected to I2C0.
 * I2C init must support multiple invocations (idempotent).
 *
 **************************************************************************************************/

/** \addtogroup g01_i2c
 *  @{
 */

/**
 * @brief Init I2C driver
 *
 * @note This function is idempotent — safe to call multiple times.
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 *      - ESP_FAIL              I2C driver installation error
 */
esp_err_t bsp_i2c_init(void);

/**
 * @brief Deinit I2C driver and free its resources
 *
 * @return
 *      - ESP_OK                On success
 *      - ESP_ERR_INVALID_ARG   I2C parameter error
 */
esp_err_t bsp_i2c_deinit(void);

/**
 * @brief Get I2C driver handle
 *
 * @note Lazily initializes I2C if not already done.
 *
 * @return I2C master bus handle
 */
i2c_master_bus_handle_t bsp_i2c_get_handle(void);

/** @} */ // end of g01_i2c

/**************************************************************************************************
 *
 * USB MSC (BSP Extension)
 *
 * The Panda Touch has a USB-A OTG host port.
 * Per BSP guide, only pinout is mandatory; USB MSC host API is provided as a BSP extension.
 *
 **************************************************************************************************/

/** \addtogroup g07_usb
 *  @{
 */

/** @brief USB event callback type */
typedef void (*bsp_usb_event_cb_t)(void);

/**
 * @brief Start USB MSC host
 *
 * Installs USB host driver, starts event task, initializes MSC client.
 * Mounts detected USB mass storage devices to /usb via VFS.
 *
 * @return
 *      - ESP_OK    On success
 *      - Else      USB host or MSC error
 */
esp_err_t bsp_usb_start(void);

/**
 * @brief Stop USB MSC host and release resources
 */
void bsp_usb_stop(void);

/**
 * @brief Check if a USB mass storage device is currently mounted
 *
 * @return true if mounted, false otherwise
 */
bool bsp_usb_is_mounted(void);

/**
 * @brief Register callback for USB device mount event
 *
 * @param[in] cb Callback function, called when a device is mounted
 */
void bsp_usb_on_mount(bsp_usb_event_cb_t cb);

/**
 * @brief Register callback for USB device unmount event
 *
 * @param[in] cb Callback function, called when a device is unmounted
 */
void bsp_usb_on_unmount(bsp_usb_event_cb_t cb);

/** @} */ // end of g07_usb

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)

/**************************************************************************************************
 *
 * LCD interface (LVGL)
 *
 * Panda Touch is shipped with an 800x480 RGB565 RGB LCD panel in DE mode.
 * LVGL is used as the graphics library. LVGL is NOT thread safe — always take the LVGL mutex
 * by calling bsp_display_lock() before calling any lv_... API, then release with bsp_display_unlock().
 *
 * Display backlight is not enabled automatically. Call bsp_display_brightness_set() after start.
 *
 **************************************************************************************************/

/** \addtogroup g04_display
 *  @{
 */

/**
 * @brief BSP display configuration structure (LVGL)
 */
typedef struct {
    lvgl_port_cfg_t lvgl_port_cfg;  /*!< LVGL port configuration */
    uint32_t        buffer_size;    /*!< Size of the buffer for the screen in pixels */
    bool            double_buffer;  /*!< True, if should be allocated two buffers */
    struct {
        unsigned int buff_dma    : 1;  /*!< Allocated LVGL buffer will be DMA capable */
        unsigned int buff_spiram : 1;  /*!< Allocated LVGL buffer will be in PSRAM */
    } flags;
} bsp_display_cfg_t;

/**
 * @brief Initialize display and start LVGL
 *
 * This function initializes the RGB LCD, configures LVGL port and starts the LVGL task.
 * LCD backlight must be enabled separately by calling bsp_display_brightness_set().
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_display_t *bsp_display_start(void);

/**
 * @brief Initialize display and start LVGL with custom config
 *
 * @param[in] cfg Display configuration
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Get pointer to input device (touch)
 *
 * @note The LVGL input device is initialized in bsp_display_start().
 *
 * @return Pointer to LVGL input device or NULL when not initialized
 */
lv_indev_t *bsp_display_get_input_dev(void);

/**
 * @brief Take LVGL mutex
 *
 * @param[in] timeout_ms Timeout in [ms]. 0 will block indefinitely.
 * @return true if mutex was taken, false on timeout
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 */
void bsp_display_unlock(void);

/**
 * @brief Rotate screen
 *
 * @param[in] disp     Pointer to LVGL display
 * @param[in] rotation Rotation angle
 */
void bsp_display_rotate(lv_display_t *disp, lv_disp_rotation_t rotation);

/**
 * @brief Put display (LCD + backlight + touch) into sleep mode
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NOT_SUPPORTED if panel does not support sleep
 */
esp_err_t bsp_display_enter_sleep(void);

/**
 * @brief Wake display (LCD + backlight + touch) from sleep mode
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_NOT_SUPPORTED if panel does not support sleep
 */
esp_err_t bsp_display_exit_sleep(void);

/** @} */ // end of g04_display

#endif // BSP_CONFIG_NO_GRAPHIC_LIB == 0

#ifdef __cplusplus
}
#endif
