//---------------------------------------------------------
// Sens_LDR.h
// Driver for GL5528 LDR (light-dependent resistor)
// Wiring: 3.3 V ── LDR ── A0 ── 10 kΩ ── GND
// (C) 2024 – Creative Commons BY-NC-SA 4.0
// Fits into the HB-UNI-Sensor1 / AskSin++ framework.
// Returns: lux() → uint16_t  (lux, integer, clamped to 0–65535)
//
// Lux calculation uses the GL5528 typical characteristic:
//   R_LDR = R_DIV * (ADC_MAX - adc) / adc          [Ω]
//   Lux   = 10 * pow(RL10 / R_LDR, 1.0/GAMMA)
// with RL10 = 10 000 Ω (resistance at 10 lux), GAMMA = 0.7
//---------------------------------------------------------

#ifndef _SENS_LDR_H_
#define _SENS_LDR_H_

#include <Arduino.h>
#include <math.h>

// GL5528 characteristic constants – adjust if your batch differs
#define LDR_RL10        10000.0f    // Ω  resistance at 10 lux
#define LDR_GAMMA       0.70f       // slope exponent from datasheet
#define LDR_R_DIV       10000.0f    // Ω  fixed voltage-divider resistor to GND
#define LDR_ADC_MAX     1023.0f     // 10-bit ADC

template<uint8_t PIN>
class Sens_LDR {

    uint16_t _lux;

public:
    Sens_LDR() : _lux(0) {}

    // Call once from initSensors()
    void init() {
        pinMode(PIN, INPUT);
        measure();   // first reading to warm up the ADC
        DPRINTLN(F("LDR init done"));
    }

    // Read ADC and convert to lux (oversampled for noise reduction)
    void measure() {
        // Average 8 samples to reduce ADC noise
        uint32_t sum = 0;
        for (uint8_t i = 0; i < 8; i++) {
            sum += (uint16_t)analogRead(PIN);
            delay(2);
        }
        uint16_t adcVal = (uint16_t)(sum / 8);

        // Complete darkness / disconnected sensor → 0 lux
        if (adcVal == 0) {
            _lux = 0;
            DPRINTLN(F("LDR: darkness (adc=0)"));
            return;
        }

        // Saturated (LDR shorted) → max lux
        if (adcVal >= (uint16_t)LDR_ADC_MAX) {
            _lux = 65535;
            return;
        }

        // R_LDR from voltage divider
        float r_ldr = LDR_R_DIV * (LDR_ADC_MAX - (float)adcVal) / (float)adcVal;

        // Lux from GL5528 power-law characteristic
        float luxF = 10.0f * powf(LDR_RL10 / r_ldr, 1.0f / LDR_GAMMA);

        // Clamp and store
        if (luxF < 0.0f)          luxF = 0.0f;
        if (luxF > 65535.0f)      luxF = 65535.0f;
        _lux = (uint16_t)luxF;

        DPRINT(F("LDR adc/R_ldr/lux = "));
        DDEC(adcVal); DPRINT(F(" / "));
        DPRINT(F("(float) / "));
        DDECLN(_lux);
    }

    // Accessor expected by main sketch / message builder
    uint16_t lux() const { return _lux; }
};

#endif  // _SENS_LDR_H_
