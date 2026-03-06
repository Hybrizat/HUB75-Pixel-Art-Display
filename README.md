# HUB75 Pixel Art Display

A Wi-Fi-enabled LED Pixel Art Display based on HUB75E panels, powered by an ESP32.

Based on [mzashh's project](https://github.com/mzashh/HUB75-Pixel-Art-Display)

---

## Changelog

### Version 0.1.0a (Major Update)
- Added **Multiple Panel Support**.
- Added **UTF-8 Support** for Scrolling Text.
- Better look WebUI.

---

## New Features

- **Multiple Panel Support**: You can link multiple panels and use different arrangements.
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/test1.jpg" width="823">
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/test2.jpg" width="823">

- **UTF-8 Character Support**: Scrolling text now support UTF-8 Charactors (depends on the font you are using). Default using
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/test_cn.jpg" width="823">
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/test_jp.jpg" width="823">

- **WebUI**:
  - Added a color selection palette.
  - Can directly input hexadecimal color codes.
  - Added color preview.
  - Support direct input of brightness values.
  - Set the color to dark mode.

<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/webpage.png" width="823">
</p>

---

## Known issues

> Yes, I know there are several problems, but I don't know how to fix it for now..

- If both "Play GIFs" and "Scrolling Text" are turned on at the same time, serious flickering will occur. The buttons on the WebUI have been set to be mutually exclusive.
- If only the scan lines are displayed:
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/error.jpg" width="823">
</p>
Please set the "Core Debugging Level" in the Arduino IDE to any option other than "None". I have no idea what caused this, but enable core debugging can make it work normally.
<p align="center">
<img src="https://github.com/Hybrizat/HUB75-Pixel-Art-Display/blob/main/images/solution.png" width="823">
</p>

- The scroll speed will be slow if the text is too long
- GIFs do not scale at different resolutions (it's working as designed. I didn't add a "scaling" function. )

---

## Hardware Requirements

- **HUB75 Panel**: Compatible with the [ESP32-HUB75-MatrixPanel-DMA library](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA). Tested on two 64x32 1/16 scan panel.
- **ESP32 Board**: Tested only on ESP32, other models may work.

### Pin Configuration

> **Note**: GPIO 34+ on the ESP32 are input-only. GPIO 5, 23, 18, and 19 are reserved for future SD card support. If a 1/16 scan panel is used , pin E can set to -1 (disabled).

| Panel | ESP32 GPIO Pin |
|--------|----------|
| R1     | 25       |
| G1     | 26       |
| B1     | 27       |
| R2     | 14       |
| G2     | 12       |
| B2     | 13       |
| A      | 22       |
| B      | 32       |
| C      | 33       |
| D      | 17       |
| E      | 21       |
| OE     | 15       |
| CLK    | 16       |
| LAT    | 4        |

---

## Firmware

### Required Libraries
- [ESP32-HUB75-MatrixPanel-DMA](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA)
- [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF)
- [GFX_Lite](https://github.com/mrcodetastic/GFX_Lite)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [U8g2_for_Adafruit_GFX](https://github.com/olikraus/U8g2_for_Adafruit_GFX)

### Configuration
- **Wi-Fi**:
  - Change the Wi-Fi SSID and password in `firmware.ino` (only 2.4GHz networks are supported).
- **WebUI**:
  - Update the WebUI username and password in `firmware.ino`.
- **Brightness**:
  - Set the default brightness in `firmware.ino` (range: 0–255).
- **Panel Resolution**:
  - Set the resolution of single panel and how they arranged in `firmware.ino`.(`PANEL_WIDTH` `PANEL_HEIGHT` `PANEL_COLS` `PANEL_ROWS` and `CHAIN_TYPE`)
- **Authentication**:
  - Disable authentication by changing `false` to `true` on line 258 in `webserver.ino`.
- **Fonts**
  - using `u8g2_font_wqy16_t_gb2312` by default. Supports most commonly used Chinese characters, Hiragana, and Katakana. If you want to use Japnese Kanji or Korean please select a [different font](https://github.com/olikraus/u8g2/blob/master/doc/u8g2fntlistall.pdf)

---

## Case and Assembly

> Visit the original author's website for more information.

- **3D Printable Files for the Case and the Diffuser**:
  - [Printables](https://www.printables.com/model/875329-hub75-pixel-art-display-case)
- **Project Documentation and Assembly**:
  - [Mzashh's Website](https://mzashh.weebly.com/pixel-art-display.html)
   

## Declaration

- This project was completed with the assistance of AI.
