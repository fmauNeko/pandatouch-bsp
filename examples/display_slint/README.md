# display_slint

Slint UI example for the BigTreeTech Panda Touch BSP.

Replicates the `display_demo` example using [Slint](https://slint.dev/) instead of LVGL for the UI framework.

Three-tab Slint UI demonstrating all hardware capabilities:

| Tab | Feature |
|-----|---------|
| Backlight | Slider to set PWM brightness (1–100%) |
| USB | File browser — lists files and directories from an inserted USB drive |
| Sensor | Live temperature & humidity from the optional Panda Sense AHT30 module |

The AHT30 sensor is optional. If not connected the Sensor tab shows a "not connected" message.

## Prerequisites

### Rust esp-rs Toolchain

Slint compiles from source using Cargo. Install the Rust Xtensa toolchain:

```bash
cargo install espup
espup install
. $HOME/export-esp.sh
```

See [esp-rs/espup](https://github.com/esp-rs/espup) for details.

### pandatouch_noglib Component

The `pandatouch_noglib` component is not stored in the repository. Generate it first:

```bash
pip install idf-component-manager==2.* py-markdown-table
python .github/ci/bsp_noglib.py pandatouch
```

### ESP-IDF

ESP-IDF v5.3 or later is required.

## Hardware

| Module | Interface | GPIO |
|--------|-----------|------|
| GT911 touch | I2C0 | SCL=1, SDA=2 |
| Backlight | LEDC PWM | GPIO 21 |
| USB-A host | USB OTG | D+=20, D-=19 |
| Panda Sense AHT30 | I2C1 | SCL=3, SDA=4 |

## Build

```bash
cd examples/display_slint
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

## Differences from display_demo

| Aspect | display_demo | display_slint |
|--------|-------------|---------------|
| UI framework | LVGL (C) | Slint (C++) |
| BSP component | `pandatouch` | `pandatouch_noglib` |
| Language | C | C++ |
| Build requirement | ESP-IDF only | ESP-IDF + Rust Xtensa toolchain |

## License

Slint is available under GPL-3.0-only, LicenseRef-Slint-Royalty-free-2.0, or LicenseRef-Slint-Software-3.0.
This example code is MIT licensed.
