/*
 * SPDX-FileCopyrightText: 2026 fmauNeko
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

// Default: LVGL included. Override by setting BSP_CONFIG_NO_GRAPHIC_LIB=1
// via pandatouch_noglib component or -DBSP_CONFIG_NO_GRAPHIC_LIB=1.
#if !defined(BSP_CONFIG_NO_GRAPHIC_LIB)
#define BSP_CONFIG_NO_GRAPHIC_LIB (0)
#endif
