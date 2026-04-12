#include "epd154.h"
#include <avr/pgmspace.h>
#include <string.h>

// ---------------------------------------------------------------------------
// 5x7 font in PROGMEM (96 chars, ASCII 32-127)
// Each entry is 5 bytes: column bitmasks, bit0 = top row, bit6 = bottom row.
// Stored in PROGMEM to save ~480 bytes of SRAM compared to a plain const array.
// ---------------------------------------------------------------------------
static const uint8_t EPD_FONT[96][5] PROGMEM = {
    {0x00,0x00,0x00,0x00,0x00}, // (space)
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x14,0x08,0x3E,0x08,0x14}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x08,0x14,0x22,0x41,0x00}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x00,0x41,0x22,0x14,0x08}, // >
    {0x02,0x01,0x51,0x09,0x06}, // ?
    {0x32,0x49,0x79,0x41,0x3E}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x3F,0x40,0x38,0x40,0x3F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x07,0x08,0x70,0x08,0x07}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x00,0x7F,0x41,0x41,0x00}, // [
    {0x02,0x04,0x08,0x10,0x20}, // backslash
    {0x00,0x41,0x41,0x7F,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04}, // ^
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x00,0x03,0x05,0x00,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x0C,0x52,0x52,0x52,0x3E}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x7F,0x10,0x28,0x44,0x00}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x20,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z
    {0x00,0x08,0x36,0x41,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00}, // }
    {0x06,0x09,0x09,0x06,0x00}, // ~ remapped -> ° (degree symbol)
};

// ---------------------------------------------------------------------------
// Hardware SPI pins (shared bus with CC1101 – only used when radio is idle)
// ---------------------------------------------------------------------------
static const uint8_t EPD_MOSI = 11;
static const uint8_t EPD_SCK  = 13;

// ---------------------------------------------------------------------------
// Constructor / init
// ---------------------------------------------------------------------------
EPD154::EPD154(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy)
    : _cs(cs), _dc(dc), _rst(rst), _busy(busy), _rotation(0)
{
    memset(_lines, 0, sizeof(_lines));
}

void EPD154::init()
{
    pinMode(_cs,    OUTPUT);
    pinMode(_dc,    OUTPUT);
    pinMode(_rst,   OUTPUT);
    pinMode(_busy,  INPUT);
    pinMode(EPD_MOSI, OUTPUT);
    pinMode(EPD_SCK,  OUTPUT);

    digitalWrite(_cs,     HIGH);
    digitalWrite(EPD_SCK, LOW);
}

// ---------------------------------------------------------------------------
// Text buffer management
// ---------------------------------------------------------------------------
void EPD154::clearDisplay()
{
    memset(_lines, 0, sizeof(_lines));
}

void EPD154::printText(const char* text, uint8_t /*x*/, uint8_t y)
{
    if (y >= EPD_MAX_PAGES) return;
    strncpy(_lines[y], text, EPD_LINE_LEN - 1);
    _lines[y][EPD_LINE_LEN - 1] = '\0';
}

void EPD154::setRotation(uint8_t rot)
{
    _rotation = rot;
}

// Returns true if logical pixel (lx, ly) is black.
bool EPD154::isPixelBlack(uint8_t lx, uint8_t ly)
{
    uint8_t page = ly / (uint8_t)EPD_PAGE_H;
    uint8_t rip  = ly % (uint8_t)EPD_PAGE_H;
    if (page >= EPD_MAX_PAGES) return false;
    if (_lines[page][0] == '\0') return false;
    if (rip < EPD_TEXT_MARGIN_TOP) return false;
    uint8_t row_in_text = rip - EPD_TEXT_MARGIN_TOP;
    if (row_in_text >= (uint8_t)EPD_CHAR_H) return false;
    uint8_t font_row = row_in_text / EPD_SCALE;
    if (lx < EPD_TEXT_MARGIN_LEFT) return false;
    uint8_t cx = lx - EPD_TEXT_MARGIN_LEFT;
    uint8_t ci = cx / (uint8_t)EPD_CHAR_W;
    uint8_t cx_in_char = cx % (uint8_t)EPD_CHAR_W;
    if (ci >= (uint8_t)(EPD_LINE_LEN - 1)) return false;
    uint8_t c = (uint8_t)_lines[page][ci];
    if (c == '\0' || c < 32 || c > 127) return false;
    if (cx_in_char >= (uint8_t)(5 * EPD_SCALE)) return false; // inter-char gap
    uint8_t fc = cx_in_char / EPD_SCALE;
    uint8_t fb = pgm_read_byte(&EPD_FONT[c - 32][fc]);
    return (fb >> font_row) & 1;
}

// ---------------------------------------------------------------------------
// Low-level SPI / command helpers
// ---------------------------------------------------------------------------
void EPD154::spiWrite(uint8_t dat)
{
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(EPD_MOSI, (dat & 0x80) ? HIGH : LOW);
        dat <<= 1;
        digitalWrite(EPD_SCK, HIGH);
        digitalWrite(EPD_SCK, LOW);
    }
}

void EPD154::sendCmd(uint8_t cmd)
{
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    spiWrite(cmd);
    digitalWrite(_cs, HIGH);
}

void EPD154::sendData(uint8_t dat)
{
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    spiWrite(dat);
    digitalWrite(_cs, HIGH);
}

void EPD154::waitBusy()
{
    // SSD1681: BUSY = HIGH means busy, LOW means ready
    // delayMicroseconds() works without Timer0 ISR (safe inside noInterrupts())
    uint16_t timeout = 500; // max 5 seconds
    while (digitalRead(_busy) == HIGH && timeout > 0) {
        delayMicroseconds(10000); // 10ms, no Timer0 dependency
        timeout--;
    }
}

// ---------------------------------------------------------------------------
// SSD1681 / GDEH0154D67 initialisation
// Uses OTP waveform LUT – no need to upload a custom LUT table.
// ---------------------------------------------------------------------------
void EPD154::epd_init()
{
    // Hardware reset: LOW 10ms → HIGH 10ms (matches working_revision1 timing)
    // Required to wake from deep sleep.
    digitalWrite(_rst, LOW);  delay(10);
    digitalWrite(_rst, HIGH); delay(10);
    waitBusy();

    sendCmd(0x12); // Software reset
    delayMicroseconds(10000); // 10ms: give controller time to assert BUSY before polling
    waitBusy();
    #ifndef NDEBUG
    Serial.println(F("Init done."));
    #endif

    sendCmd(0x01); // Driver Output Control: 200 gates
    sendData(0xC7); // MUX = 199 (200 lines)
    sendData(0x00);
    sendData(0x00); // GD=0, SM=0, TB=0

    sendCmd(0x11); // Data Entry Mode: X & Y increment, counter in X direction
    sendData(0x03);

    sendCmd(0x44); // RAM X address: byte 0 to byte 24
    sendData(0x00);
    sendData(0x18);

    sendCmd(0x45); // RAM Y address: row 0 to row 199
    sendData(0x00);
    sendData(0x00);
    sendData(0xC7);
    sendData(0x00);

    sendCmd(0x3C); // Border Waveform
    sendData(0x01);

    sendCmd(0x18); // Temperature Sensor: use internal sensor
    sendData(0x80);

    sendCmd(0x4E); // Set RAM X counter to 0
    sendData(0x00);

    sendCmd(0x4F); // Set RAM Y counter to 0
    sendData(0x00);
    sendData(0x00);
}

// ---------------------------------------------------------------------------
// Row renderer: fills dst[25] with pixel data for pixel row pr.
// White = bit 1, Black = bit 0 (SSD1681 convention).
// Chars are rendered 2x scaled from the PROGMEM font.
// ---------------------------------------------------------------------------
void EPD154::renderRow(uint8_t pr, uint8_t dst[EPD_ROW_BYTES])
{
    memset(dst, 0xFF, EPD_ROW_BYTES); // all white

    if (_rotation != 0) {
        // Rotated rendering: map each physical pixel to logical coords.
        // rot=1: 90° CW  → logical (lx=pr, ly=199-phx)
        // rot=3: 90° CCW → logical (lx=199-pr, ly=phx)
        for (uint8_t phx = 0; phx < EPD_WIDTH; phx++) {
            uint8_t lx, ly;
            if (_rotation == 1) {
                lx = pr;
                ly = (EPD_HEIGHT - 1) - phx;
            } else {
                lx = (EPD_WIDTH - 1) - pr;
                ly = phx;
            }
            if (isPixelBlack(lx, ly)) {
                dst[phx >> 3] &= ~(0x80U >> (phx & 7));
            }
        }
        return;
    }

    uint8_t page = pr / (uint8_t)EPD_PAGE_H;      // 0-7
    uint8_t rip  = pr % (uint8_t)EPD_PAGE_H;      // row inside page 0-24

    // Only the middle 14 rows of each 25-row page carry text
    if (_lines[page][0] == '\0') return;
    if (rip < EPD_TEXT_MARGIN_TOP) return;
    uint8_t row_in_text = rip - EPD_TEXT_MARGIN_TOP;
    if (row_in_text >= (uint8_t)EPD_CHAR_H) return;

    uint8_t font_row = row_in_text / EPD_SCALE; // 0-6 (scaled)
    const char* text = _lines[page];

    // Walk through characters and set black pixels directly in dst[]
    uint8_t x_start = EPD_TEXT_MARGIN_LEFT;
    for (uint8_t ci = 0; text[ci] != '\0' && x_start < EPD_WIDTH; ci++, x_start += EPD_CHAR_W) {
        uint8_t c = (uint8_t)text[ci];
        if (c < 32 || c > 127) continue;

        // 5 font columns, each EPD_SCALE pixels wide, then gap
        for (uint8_t fc = 0; fc < 5; fc++) {
            uint8_t fb = pgm_read_byte(&EPD_FONT[c - 32][fc]);
            if (!((fb >> font_row) & 1)) continue; // white pixel in this column

            // EPD_SCALE scaling: write EPD_SCALE adjacent pixels
            uint8_t px_base = x_start + fc * EPD_SCALE;
            for (uint8_t s = 0; s < EPD_SCALE; s++) {
                uint8_t px = px_base + s;
                if (px >= EPD_WIDTH) break;
                // Clear the bit for this pixel (0 = black)
                dst[px >> 3] &= ~(0x80U >> (px & 7));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Full update cycle: init EPD → stream frameless render → trigger refresh → sleep
// ---------------------------------------------------------------------------
void EPD154::updateDisplay()
{
    epd_init();

    // Write BW RAM: stream all 200 rows × 25 bytes without a framebuffer
    sendCmd(0x24); // Write Black-White RAM
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);

    uint8_t row_buf[EPD_ROW_BYTES];
    for (uint8_t pr = 0; pr < EPD_HEIGHT; pr++) {
        renderRow(pr, row_buf);
        for (uint8_t bc = 0; bc < EPD_ROW_BYTES; bc++) {
            spiWrite(row_buf[bc]);
        }
    }

    digitalWrite(_cs, HIGH);
    #ifndef NDEBUG
    Serial.println(F("RAM written, triggering refresh..."));
    #endif

    // Trigger full display refresh using OTP LUT
    sendCmd(0x22);
    sendData(0xF7);
    sendCmd(0x20); // Master activation
    #ifndef NDEBUG
    Serial.print(F("BUSY before wait=")); Serial.println(digitalRead(_busy));
    #endif
    waitBusy();
    #ifndef NDEBUG
    Serial.println(F("Refresh done."));
    #endif

    // Enter deep sleep mode 1 to minimise current draw
    sendCmd(0x10);
    sendData(0x01);
}
