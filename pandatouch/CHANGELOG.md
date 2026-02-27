# Changelog

## [0.3.1](https://github.com/fmauNeko/pandatouch-bsp/compare/v0.3.0...v0.3.1) (2026-02-27)


### Bug Fixes

* **release:** correct extra-files path, sync version, fix CHANGELOG ([619c7f9](https://github.com/fmauNeko/pandatouch-bsp/commit/619c7f9aa4f918b8100a9ea22d2375d6c8aa0596))

## [0.3.0](https://github.com/fmauNeko/pandatouch-bsp/releases/tag/v0.3.0) (2026-02-27)


### Features

* **bsp:** add component skeleton, headers, Kconfig, CI workflows ([728c4b6](https://github.com/fmauNeko/pandatouch-bsp/commit/728c4b6253d9202c02b63eb1e0f4ec4287932f1f))
* **display:** add LVGL integration with esp_lvgl_port ([980d369](https://github.com/fmauNeko/pandatouch-bsp/commit/980d369eec6fdf5307df0a8549e4ec22380d56d3))
* **display:** implement low-level RGB panel and backlight ([baa9d76](https://github.com/fmauNeko/pandatouch-bsp/commit/baa9d767065e0f6257412dc3dc8b27a9608b9c8a))
* **examples:** add display_lvgl_benchmark ([89421e1](https://github.com/fmauNeko/pandatouch-bsp/commit/89421e1cb9e62761f9197e3731beec1b9fc71b31))
* **touch:** add LVGL input device integration ([be17a9f](https://github.com/fmauNeko/pandatouch-bsp/commit/be17a9f71b55ee5defd3b09cdf411e45b19c929a))
* **touch:** implement I2C bus and GT911 touch driver ([10b61b7](https://github.com/fmauNeko/pandatouch-bsp/commit/10b61b7006e44be5ad2f5af53dda7c76f50dc3c6))
* **usb:** implement USB MSC host with VFS mount ([ec77df6](https://github.com/fmauNeko/pandatouch-bsp/commit/ec77df650d8a0a3407511ed8ff4ccc0fb5f594a3))


### Bug Fixes

* **bsp_usb:** decouple STOP notification from task creation ([06936c0](https://github.com/fmauNeko/pandatouch-bsp/commit/06936c02bad8483a9cec9f92369c8844fcf986b1))
* **bsp:** add ret_panel null guard and s_display null check in bsp_display_new ([e843904](https://github.com/fmauNeko/pandatouch-bsp/commit/e8439044a51fe99c487e16ae6c320fa48a5bd95b))
* **bsp:** remove direct lvgl dep, let esp_lvgl_port manage it ([bd0e1ab](https://github.com/fmauNeko/pandatouch-bsp/commit/bd0e1ab9ef7b227aa351f9db8733e18be8c7c8e3))
* **bsp:** reset s_usb_host_shutdown on start and self-delete msc_app_task ([d55276f](https://github.com/fmauNeko/pandatouch-bsp/commit/d55276f3de6cd23844214e128dc1b874012d0d3e))
* **build:** resolve IDF 6.0 compile errors ([eb811b3](https://github.com/fmauNeko/pandatouch-bsp/commit/eb811b3aef47988b9bde270a2c3b8427b27d7634))
* **display_demo:** fix WDT crash, screen tearing, and USB remount failure ([3610835](https://github.com/fmauNeko/pandatouch-bsp/commit/361083559228b73e7fed62baf3d3b72e49bc6026))
* **display:** correct color output and eliminate tearing ([dc97067](https://github.com/fmauNeko/pandatouch-bsp/commit/dc9706708a3e90772c6e88436d6e4ff6285a5259))
* **doc:** hyphenate ESP32-S3-based as compound adjective in HARDWARE.md ([d74ac20](https://github.com/fmauNeko/pandatouch-bsp/commit/d74ac20b68db774a170474858b0375dcce3b2cb5))
* **examples:** check bsp_usb_start() return value and improve display_hello README; add dependency markers to pandatouch README ([87efde8](https://github.com/fmauNeko/pandatouch-bsp/commit/87efde8dd7a7de3613f5f330f81c461f24f45792))
* multiple fixes ([93e6762](https://github.com/fmauNeko/pandatouch-bsp/commit/93e6762e5a640bd504620bc91e6b32b32b7f3464))
* **noglib:** add missing esp_lcd_panel_ops.h, allocate fb in PSRAM ([6a86d63](https://github.com/fmauNeko/pandatouch-bsp/commit/6a86d63d7c6d674545b906768ae383bcc1fd7a5f))
* **pandatouch:** align README license with SPDX header (MIT) ([1c81416](https://github.com/fmauNeko/pandatouch-bsp/commit/1c81416a1a781c6efa5fb259919f0689c2b8cc83))
* **pandatouch:** fix double-free, race, and task handle leak in bsp_usb_stop/start ([85501ec](https://github.com/fmauNeko/pandatouch-bsp/commit/85501eced7b95e033a7b5d7c49bee3ed93036ced))
* **pandatouch:** fix i2c init guards, LVGL version compat, and USB resource cleanup on failure ([cef3987](https://github.com/fmauNeko/pandatouch-bsp/commit/cef3987496cf01871a15ca25a684ec4f498c3f45))
* **pandatouch:** fix USB hotplug remount via owned event handler task ([a38bf5a](https://github.com/fmauNeko/pandatouch-bsp/commit/a38bf5adf4d1016311aa8cc59cca09eb547402fd))
* **pandatouch:** remove dead BSP_CAPS_*=0 defines and overhaul README to esp-bsp conventions ([cb27f8c](https://github.com/fmauNeko/pandatouch-bsp/commit/cb27f8c7ac4c6c00a212118455ebf09aa0707aae))
* **pandatouch:** use #ifndef consistently in config.h and add missing includes in bsp_err_check.h ([90edd86](https://github.com/fmauNeko/pandatouch-bsp/commit/90edd8610a8acd78b401e9eb74041ee0eddb2b32))
* **touch:** pass driver_data to GT911 for I2C address selection ([ddf3af1](https://github.com/fmauNeko/pandatouch-bsp/commit/ddf3af192faa716968bf5a70f3bc8503ef45d15d))
* **touch:** set scl_speed_hz on GT911 I2C device config ([db06b8e](https://github.com/fmauNeko/pandatouch-bsp/commit/db06b8e8e890dba0434ac477e66439f9ee481c70))
