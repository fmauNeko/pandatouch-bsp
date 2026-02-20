/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_err.h"
#include "esp_log.h"

#ifdef CONFIG_BSP_ERROR_CHECK

#define BSP_ERROR_CHECK_RETURN_ERR(x) do {                      \
        esp_err_t err_rc_ = (x);                                \
        if (unlikely(err_rc_ != ESP_OK)) {                      \
            ESP_LOGE("BSP", "BSP error %d at %s:%d", err_rc_,   \
                     __FILE__, __LINE__);                        \
            abort();                                             \
        }                                                        \
    } while(0)

#define BSP_ERROR_CHECK_RETURN_NULL(x) do {                     \
        esp_err_t err_rc_ = (x);                                \
        if (unlikely(err_rc_ != ESP_OK)) {                      \
            ESP_LOGE("BSP", "BSP error %d at %s:%d", err_rc_,   \
                     __FILE__, __LINE__);                        \
            abort();                                             \
        }                                                        \
    } while(0)

#define BSP_NULL_CHECK(x, ret) do {                             \
        if ((x) == NULL) {                                      \
            ESP_LOGE("BSP", "BSP null at %s:%d",                \
                     __FILE__, __LINE__);                        \
            abort();                                             \
        }                                                        \
    } while(0)

#else // CONFIG_BSP_ERROR_CHECK

#define BSP_ERROR_CHECK_RETURN_ERR(x) do {                      \
        esp_err_t err_rc_ = (x);                                \
        if (unlikely(err_rc_ != ESP_OK)) {                      \
            return err_rc_;                                      \
        }                                                        \
    } while(0)

#define BSP_ERROR_CHECK_RETURN_NULL(x) do {                     \
        esp_err_t err_rc_ = (x);                                \
        if (unlikely(err_rc_ != ESP_OK)) {                      \
            return NULL;                                         \
        }                                                        \
    } while(0)

#define BSP_NULL_CHECK(x, ret) do {                             \
        if ((x) == NULL) {                                      \
            return ret;                                          \
        }                                                        \
    } while(0)

#endif // CONFIG_BSP_ERROR_CHECK
