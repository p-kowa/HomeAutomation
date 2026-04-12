#ifndef EPD154_H
#define EPD154_H

// Minimal driver for WeAct 1.54" 200x200 BW e-paper (SSD1681 / GDEH0154D67)
// No framebuffer: text is rendered row-by-row and streamed to the display.
// Font: 5x7 stored in PROGMEM, rendered 2x scaled (10x14 px per character).
// SPI pins: MOSI=11, SCK=13 (shared with CC1101, safe to use when radio is idle).
// Unique pins: CS, DC, RST, BUSY (define in Device_BME280.h as EPD_CS/DC/RST/BUSY).

#include <Arduino.h>

#define EPD_WIDTH      200
#define EPD_HEIGHT     200
#define EPD_ROW_BYTES  25      // 200 / 8

// Display layout constants (3x scaled 5x7 font)
#define EPD_SCALE           3
#define EPD_CHAR_W          18  // (5 cols + 1 gap) * scale
#define EPD_CHAR_H          21  // 7 rows * scale
#define EPD_PAGE_H          40  // pixel rows per page (200 / 5 pages)
#define EPD_TEXT_MARGIN_TOP  9  // top margin within page in pixels
#define EPD_TEXT_MARGIN_LEFT 4  // left margin in pixels
#define EPD_MAX_PAGES        5
#define EPD_LINE_LEN        12  // max chars per line (10 visible + null)

class EPD154 {
public:
    EPD154(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy);

    // Set up GPIO pins (call once in Arduino setup())
    void init();

    // Clear the text buffer
    void clearDisplay();

    // Store a text string for page y (0-7). x is ignored (fixed left margin).
    void printText(const char* text, uint8_t x, uint8_t y);

    // Set display rotation: 0 = normal, 1 = 90° CW, 3 = 90° CCW
    void setRotation(uint8_t rot);

    // Initialise EPD, render text buffer, trigger full refresh, then deep-sleep.
    // Call this after all printText() calls for the current frame.
    void updateDisplay();

private:
    uint8_t _cs, _dc, _rst, _busy;
    uint8_t _rotation;
    char    _lines[EPD_MAX_PAGES][EPD_LINE_LEN];

    void epd_init();          // hardware reset + register init
    void sendCmd(uint8_t cmd);
    void sendData(uint8_t dat);
    void waitBusy();
    void spiWrite(uint8_t dat);

    // Returns true if logical pixel (lx, ly) is black (used for rotated render).
    bool isPixelBlack(uint8_t lx, uint8_t ly);

    // Render 25 bytes for pixel row pr into dst[] (all-white background).
    void renderRow(uint8_t pr, uint8_t dst[EPD_ROW_BYTES]);
};

#endif // EPD154_H
