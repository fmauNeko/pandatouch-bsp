# Hardware Reference: BigTreeTech Panda Touch

The BigTreeTech Panda Touch is an ESP32-S3 based device with a large high-resolution display and USB OTG capabilities.

## MCU Specifications

- **SoC**: ESP32-S3 (Dual-core Xtensa LX7 @ 240 MHz)
- **Flash**: 16 MB QSPI
- **PSRAM**: 8 MB Octal
- **Wireless**: Wi-Fi 2.4 GHz + Bluetooth 5 (LE)

## Display Specifications

- **Type**: 5.0" (or 7.0") TFT LCD
- **Resolution**: 800 × 480
- **Color Format**: RGB565 (16-bit)
- **Interface**: 16-bit Parallel RGB
- **Mode**: DE (Data Enable) mode
- **Pixel Clock**: 23 MHz (typical)
- **Backlight**: LEDC PWM controlled (GPIO 21)

## Touch Controller

- **Chip**: GT911 (Capacitive)
- **Interface**: I2C0
- **I2C Address**: 0x5D (or 0x14)
- **Interrupt**: GPIO 40
- **Reset**: GPIO 41

## USB Interface

- **USB-C Port**: Power + Serial Debug (CH340K bridge)
- **USB-A Port**: Native USB OTG Host (GPIO 19/20)
- **Function**: Mass Storage Class (MSC) support

## Pinout Mapping

| Signal | GPIO | Function |
|--------|------|----------|
| **LCD_PCLK** | 5 | Pixel Clock |
| **LCD_DE** | 38 | Data Enable |
| **LCD_RST** | 46 | Display Reset |
| **LCD_DATA0** | 17 | B3 |
| **LCD_DATA1** | 18 | B4 |
| **LCD_DATA2** | 48 | B5 |
| **LCD_DATA3** | 47 | B6 |
| **LCD_DATA4** | 39 | B7 |
| **LCD_DATA5** | 11 | G2 |
| **LCD_DATA6** | 12 | G3 |
| **LCD_DATA7** | 13 | G4 |
| **LCD_DATA8** | 14 | G5 |
| **LCD_DATA9** | 15 | G6 |
| **LCD_DATA10** | 16 | G7 |
| **LCD_DATA11** | 6 | R3 |
| **LCD_DATA12** | 7 | R4 |
| **LCD_DATA13** | 8 | R5 |
| **LCD_DATA14** | 9 | R6 |
| **LCD_DATA15** | 10 | R7 |
| **BACKLIGHT** | 21 | PWM |
| **I2C0_SCL** | 1 | Touch SCL |
| **I2C0_SDA** | 2 | Touch SDA |
| **TOUCH_INT** | 40 | Touch INT |
| **TOUCH_RST** | 41 | Touch RST |
| **I2C1_SCL** | 3 | Expansion SCL |
| **I2C1_SDA** | 4 | Expansion SDA |
| **USB_D-** | 19 | USB Host D- |
| **USB_D+** | 20 | USB Host D+ |
| **UART0_TX** | 43 | Debug TX |
| **UART0_RX** | 44 | Debug RX |
