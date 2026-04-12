//---------------------------------------------------------
// Sens_AHT11.h
// Tiny I2C driver for AHT11 temperature / humidity sensor
// (C) 2024 – Creative Commons BY-NC-SA 4.0
// Fits into the HB-UNI-Sensor1 / AskSin++ framework.
// Returns:  temperature() → int16_t  (°C × 10,  e.g. 235 = 23.5 °C)
//           humidity()    → uint16_t (% × 10,   e.g. 650 = 65.0 %)
//---------------------------------------------------------

#ifndef _SENS_AHT11_H_
#define _SENS_AHT11_H_

#include <Wire.h>

// AHT11 fixed I2C address
#define AHT11_ADDR       0x38

// Commands
#define AHT11_CMD_INIT   0xE1
#define AHT11_CMD_TRIG   0xAC
#define AHT11_STATUS_BUSY  0x80

class Sens_AHT11 {

    int16_t  _temperature10;   // °C × 10
    uint16_t _humidity10;      // % × 10
    int8_t   _tempOffset10;    // calibration offset °C × 10
    int8_t   _humidOffset10;   // calibration offset % × 10
    bool     _initialized;

    // Send a 3-byte command over I2C
    void sendCmd(uint8_t b0, uint8_t b1, uint8_t b2) {
        Wire.beginTransmission(AHT11_ADDR);
        Wire.write(b0);
        Wire.write(b1);
        Wire.write(b2);
        Wire.endTransmission();
    }

    bool isBusy() {
        Wire.requestFrom((uint8_t)AHT11_ADDR, (uint8_t)1);
        if (Wire.available()) {
            return (Wire.read() & AHT11_STATUS_BUSY) != 0;
        }
        return true;
    }

public:
    Sens_AHT11()
        : _temperature10(0), _humidity10(0),
          _tempOffset10(0), _humidOffset10(0), _initialized(false) {}

    // Call once from initSensors().
    // tempOffset10  … temperature offset in 0.1 K steps (subtracted)
    // humidOffset10 … humidity offset in 0.1 % steps (added)
    void init(int8_t tempOffset10 = 0, int8_t humidOffset10 = 0) {
        _tempOffset10  = tempOffset10;
        _humidOffset10 = humidOffset10;

        Wire.begin();
        delay(40);                                  // power-on stabilisation

        // Soft-reset
        Wire.beginTransmission(AHT11_ADDR);
        Wire.write(0xBA);                           // soft reset command
        Wire.endTransmission();
        delay(20);

        // Initialise / calibrate
        sendCmd(AHT11_CMD_INIT, 0x08, 0x00);
        delay(10);

        _initialized = true;
        DPRINTLN(F("AHT11 init done"));
    }

    // Trigger measurement and read result.
    // Safe to call repeatedly; takes ≈80 ms (blocking).
    void measure() {
        if (!_initialized) return;

        sendCmd(AHT11_CMD_TRIG, 0x33, 0x00);
        delay(80);                                  // conversion time

        // Wait until not busy (max extra 20 ms)
        uint8_t timeout = 20;
        while (isBusy() && timeout--) {
            delay(1);
        }

        // Read 6 bytes
        uint8_t data[6] = { 0 };
        Wire.requestFrom((uint8_t)AHT11_ADDR, (uint8_t)6);
        if (Wire.available() < 6) {
            DPRINTLN(F("AHT11 read error"));
            return;
        }
        for (uint8_t i = 0; i < 6; i++) {
            data[i] = Wire.read();
        }

        // Parse humidity (20-bit raw)
        uint32_t rawHumid = ((uint32_t)data[1] << 12)
                          | ((uint32_t)data[2] <<  4)
                          | ((uint32_t)data[3] >>  4);

        // Parse temperature (20-bit raw)
        uint32_t rawTemp  = ((uint32_t)(data[3] & 0x0F) << 16)
                          | ((uint32_t)data[4] <<  8)
                          |  (uint32_t)data[5];

        // Convert – results in 0.1 °C / 0.1 % steps
        // T [°C]  = rawTemp  / 1048576 * 200 - 50
        // H [%]   = rawHumid / 1048576 * 100
        int32_t  tempCalc  = (int32_t)((rawTemp  * 2000UL) >> 20) - 500;  // 0.1 °C
        uint32_t humidCalc = (rawHumid * 1000UL) >> 20;                   // 0.1 %

        // Apply offsets from WebUI
        _temperature10 = (int16_t)(tempCalc  - _tempOffset10);
        _humidity10    = (uint16_t)(humidCalc + _humidOffset10);

        // Clamp humidity to 0-1000 (0–100 %)
        if (_humidity10 > 1000) _humidity10 = 1000;

        DPRINT(F("AHT11 T/H (x10) = "));
        DDEC(_temperature10); DPRINT(F(" / ")); DDECLN(_humidity10);
    }

    // Accessors expected by the main sketch / message builder
    int16_t  temperature() const { return _temperature10; }
    uint16_t humidity()    const { return _humidity10;    }
};

#endif  // _SENS_AHT11_H_
