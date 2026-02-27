# BSP: Panda Touch

| [HW Reference](doc/HARDWARE.md) | [HOW TO USE API](include/bsp/pandatouch.h) | [EXAMPLES](#compatible-bsp-examples) | [![Component Registry](https://components.espressif.com/components/fmauneko/pandatouch/badge.svg)](https://components.espressif.com/components/fmauneko/pandatouch) | ![maintenance-status](https://img.shields.io/badge/maintenance-actively--developed-brightgreen.svg) |
| ------------------------------- | ------------------------------------------ | ------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------- |

## Overview

<table>
<tr><td>

The BigTreeTech Panda Touch is an ESP32-S3 based device designed for 3D printer control and monitoring, featuring a large 5-inch 800x480 RGB LCD and capacitive touch.

The Panda Touch comes with 16MB of QSPI flash and 8MB of Octal PSRAM. It is equipped with a variety of peripherals, such as a GT911 capacitive touch screen, a USB-A host port for mass storage, and an expansion header for additional sensors.

The device uses a Type-C USB connector for power input, serial debugging, and programming.

</td><td width="200" valign="top">
  <img src="https://bigtreetech.com/cdn/shop/files/1_15_800x.jpg?v=1704358814">
</td></tr>
</table>

## Capabilities and dependencies

<div align="center">
<!-- START_DEPENDENCIES -->

|     Available    |       Capability       |Controller/Codec|                                                 Component                                                |Version|
|------------------|------------------------|----------------|----------------------------------------------------------------------------------------------------------|-------|
|:heavy_check_mark:|     :pager: DISPLAY    |       rgb      |                                                    idf                                                   | >=5.3 |
|:heavy_check_mark:|:black_circle: LVGL_PORT|                |      [espressif/esp_lvgl_port](https://components.espressif.com/components/espressif/esp_lvgl_port)      |   ^2  |
|:heavy_check_mark:|    :point_up: TOUCH    |      gt911     |[espressif/esp_lcd_touch_gt911](https://components.espressif.com/components/espressif/esp_lcd_touch_gt911)|   ^1  |
|:heavy_check_mark:|  :floppy_disk: USB_MSC |                |       [espressif/usb_host_msc](https://components.espressif.com/components/espressif/usb_host_msc)       |   ^1  |

<!-- END_DEPENDENCIES -->
</div>

## Compatible BSP Examples

<div align="center">
<!-- START_EXAMPLES -->

| Example | Description | Try with ESP Launchpad |
| ------- | ----------- | ---------------------- |
| [display_demo — Panda Touch showcase](https://github.com/fmauneko/pandatouch-bsp/tree/master/examples/display_demo) | Three-tab LVGL UI demonstrating backlight control, USB file browser, and optional AHT30 sensor data. | - |
| [Hello Panda Touch](https://github.com/fmauneko/pandatouch-bsp/tree/master/examples/display_hello) | Simple "Hello World" example showing a centered label and USB MSC host initialization. | - |

<!-- END_EXAMPLES -->
</div>

<!-- START_BENCHMARK -->
<!-- END_BENCHMARK -->

## License

MIT