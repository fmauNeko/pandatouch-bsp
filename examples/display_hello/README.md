# display_hello

LVGL example for the BigTreeTech Panda Touch BSP.

Displays a "Hello, Panda Touch!" label centered on screen and starts the USB MSC host.

## Build

Requires ESP-IDF v6.0 or later.

```bash
cd examples/display_hello
idf.py set-target esp32s3
idf.py build flash monitor
```

## Expected output

- Display: centered "Hello, Panda Touch!" label on a blank screen.
- Serial monitor: USB MSC host initialisation messages, and on USB drive insertion a mount success log such as `I (xxx) bsp_usb: USB MSC mounted at /usb`.
