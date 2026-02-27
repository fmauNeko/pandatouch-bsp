# display_demo

Comprehensive BSP showcase for the BigTreeTech Panda Touch.

Three-tab LVGL UI demonstrating all hardware capabilities:

| Tab | Feature |
|-----|---------|
| Backlight | Slider to set PWM brightness (1–100%) |
| USB | File browser — lists files and directories from an inserted USB drive |
| Sensor | Live temperature & humidity from the optional Panda Sense AHT30 module |

The AHT30 sensor is optional. If not connected the Sensor tab shows a "not connected" message.

## Hardware

| Module | Interface | GPIO |
|--------|-----------|------|
| GT911 touch | I2C0 | SCL=1, SDA=2 |
| Backlight | LEDC PWM | GPIO 21 |
| USB-A host | USB OTG | D+=20, D-=19 |
| Panda Sense AHT30 | I2C1 | SCL=3, SDA=4 |

## Build

```bash
cd examples/display_demo
idf.py set-target esp32s3
idf.py build flash monitor
```
