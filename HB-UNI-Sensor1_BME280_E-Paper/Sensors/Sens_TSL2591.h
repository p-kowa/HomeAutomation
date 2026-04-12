
#ifndef _SENS_TSL2591_H_
#define _SENS_TSL2591_H_

#include <Wire.h>
#include <Sensors.h>

// TSL2591 I2C address (fixed, cannot be changed)
#define TSL2591_ADDR        0x29
#define TSL2591_CMD         0xA0  // command bit for register access

// Registers
#define TSL2591_REG_ENABLE  0x00
#define TSL2591_REG_CONFIG  0x01
#define TSL2591_REG_ID      0x12
#define TSL2591_REG_STATUS  0x13
#define TSL2591_REG_C0L     0x14  // channel 0 low byte  (visible + IR)
#define TSL2591_REG_C1L     0x16  // channel 1 low byte  (IR only)

// Config: integration time (lower 3 bits)
#define TSL2591_INTEGR_100MS  0x00  // 100 ms

// Config: gain (bits 5:4)
#define TSL2591_GAIN_LOW      0x00  //   1x  — bright light / outdoor
#define TSL2591_GAIN_MED      0x10  //  25x  — normal indoor light
#define TSL2591_GAIN_HIGH     0x20  // 428x  — dim light / twilight

// Adafruit lux formula coefficients
#define TSL2591_LUX_DF    408.0F
#define TSL2591_LUX_COEFB   1.64F
#define TSL2591_LUX_COEFC   0.59F
#define TSL2591_LUX_COEFD   0.86F

namespace as {

class Sens_TSL2591 : public Sensor {

    uint16_t _lux;
    uint8_t  _gain;    // TSL2591_GAIN_xxx

    // -----------------------------------------------------------------------
    static void writeReg(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(TSL2591_ADDR);
        Wire.write(TSL2591_CMD | reg);
        Wire.write(val);
        Wire.endTransmission();
    }

    static uint8_t readReg8(uint8_t reg) {
        Wire.beginTransmission(TSL2591_ADDR);
        Wire.write(TSL2591_CMD | reg);
        Wire.endTransmission();
        Wire.requestFrom((uint8_t)TSL2591_ADDR, (uint8_t)1);
        return Wire.read();
    }

    static uint16_t readReg16(uint8_t reg) {
        Wire.beginTransmission(TSL2591_ADDR);
        Wire.write(TSL2591_CMD | reg);
        Wire.endTransmission();
        Wire.requestFrom((uint8_t)TSL2591_ADDR, (uint8_t)2);
        uint16_t val = (uint16_t)Wire.read();
        val |= ((uint16_t)Wire.read() << 8);
        return val;
    }

public:
    Sens_TSL2591() : _lux(0), _gain(TSL2591_GAIN_MED) {}

    // Call AFTER Wire.begin() (e.g. after BME280.init)
    void init() {
        uint8_t id = readReg8(TSL2591_REG_ID);
        if (id != 0x50) {
            DPRINT(F("TSL2591 NOT found, ID=0x")); DDECLN(id);
            return;
        }
        _present = true;
        // leave sensor disabled until measure(), configure gain + 100ms integration
        writeReg(TSL2591_REG_CONFIG, TSL2591_INTEGR_100MS | _gain);
        DPRINTLN(F("TSL2591 OK"));
    }

    void measure() {
        if (!_present) return;

        // Enable: power on + ALS
        writeReg(TSL2591_REG_ENABLE, 0x03);
        delay(120);  // 100 ms integration + 20 ms margin

        // Auto-gain: if channel 0 is saturated (>= 0xFFFF) drop gain
        _gain = TSL2591_GAIN_MED;
        writeReg(TSL2591_REG_CONFIG, TSL2591_INTEGR_100MS | _gain);

        uint16_t ch0 = readReg16(TSL2591_REG_C0L);
        uint16_t ch1 = readReg16(TSL2591_REG_C1L);

        if (ch0 >= 0xFFFF) {
            // saturated — switch to low gain and re-measure
            _gain = TSL2591_GAIN_LOW;
            writeReg(TSL2591_REG_CONFIG, TSL2591_INTEGR_100MS | _gain);
            delay(120);
            ch0 = readReg16(TSL2591_REG_C0L);
            ch1 = readReg16(TSL2591_REG_C1L);
        }

        // Disable sensor to save power
        writeReg(TSL2591_REG_ENABLE, 0x00);

        // Lux formula (Adafruit, fixed 100ms integration)
        // cpl = (atime * again) / LUX_DF
        float again = (_gain == TSL2591_GAIN_LOW) ? 1.0F : 25.0F;
        float cpl   = (100.0F * again) / TSL2591_LUX_DF;

        float lux1 = ((float)ch0 - (TSL2591_LUX_COEFB * (float)ch1)) / cpl;
        float lux2 = ((TSL2591_LUX_COEFC * (float)ch0) - (TSL2591_LUX_COEFD * (float)ch1)) / cpl;
        float lux  = (lux1 > lux2) ? lux1 : lux2;
        if (lux < 0.0F)         lux = 0.0F;
        if (lux > 65535.0F)     lux = 65535.0F;
        _lux = (uint16_t)lux;

        DPRINT(F("TSL2591 ch0/ch1/lux = "));
        DDEC(ch0); DPRINT(F("/")); DDEC(ch1); DPRINT(F("/"));
        DDECLN(_lux);
    }

    uint16_t lux() { return _lux; }
};

} // namespace as

#endif
