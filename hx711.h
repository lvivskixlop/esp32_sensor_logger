#pragma once

#include <Arduino.h>

class HX711Custom {
public:
    // ctor: doutPin = DOUT (DATA), sckPin = PD_SCK (CLOCK)
    // gain: 128 (default), 64, or 32
    HX711Custom(uint8_t doutPin, uint8_t sckPin, uint16_t gain = 128)
        : doutPin(doutPin), sckPin(sckPin), scale(1.0f), offset(0) {
        setGain(gain);
    }

    // configure pins; call once in setup()
    void begin() {
        pinMode(sckPin, OUTPUT);
        digitalWrite(sckPin, LOW); // clock idle low
        pinMode(doutPin, INPUT);
    }

    // Returns true if HX711 DRDY (DOUT) is low (data ready)
    bool is_ready() {
        return digitalRead(doutPin) == LOW;
    }

    // Blocking raw read. Waits up to timeout_ms for device ready.
    // Returns signed 32-bit value representing 24-bit two's complement result.
    // Throws no exceptions; on timeout returns LONG_MIN.
    long readRaw(unsigned long timeout_ms = 1000) {
        unsigned long start = millis();
        while (!is_ready()) {
            if ((millis() - start) > timeout_ms) {
                return LONG_MIN; // timeout
            }
            yield();
        }

        uint32_t value = 0;
        // read 24 bits MSB first
        for (uint8_t i = 0; i < 24; ++i) {
            digitalWrite(sckPin, HIGH);
            // min high time ~1us, use a small delay to be safe
            delayMicroseconds(1);
            value = (value << 1) | (digitalRead(doutPin) ? 1 : 0);
            digitalWrite(sckPin, LOW);
            delayMicroseconds(1);
        }

        // set gain & channel for next measurement by sending extra pulses:
        for (uint8_t i = 0; i < gainPulses; ++i) {
            digitalWrite(sckPin, HIGH);
            delayMicroseconds(1);
            digitalWrite(sckPin, LOW);
            delayMicroseconds(1);
        }

        // sign extend 24-bit to 32-bit signed
        if (value & 0x800000) {
            value |= 0xFF000000;
        }
        return (long)value;
    }

    // Read raw value averaged over 'times' samples (skips invalid reads)
    long readAverage(uint8_t times = 10) {
        long sum = 0;
        uint8_t readCount = 0;
        for (uint8_t i = 0; i < times; ++i) {
            long val = readRaw(1000);
            if (val == LONG_MIN) {
                // skip this sample
                continue;
            }
            sum += val;
            ++readCount;
        }
        if (readCount == 0) return LONG_MIN;
        return sum / readCount;
    }

    // Tare: sample 'times' and set offset to that average
    void tare(uint8_t times = 10) {
        long avg = readAverage(times);
        if (avg != LONG_MIN) {
            offset = avg;
        }
    }

    // Set calibration scale (units per ADC count). Use calibrate() helper in example.
    void setScale(float scaleFactor) {
        scale = scaleFactor;
    }
    float getScale() const { return scale; }

    void setOffset(long offs) { offset = offs; }
    long getOffset() const { return offset; }

    // Return value in units ( (raw - offset) / scale )
    // Returns NAN if read fails.
    float getUnits(uint8_t times = 5) {
        long raw = readAverage(times);
        if (raw == LONG_MIN) return NAN;
        return (raw - offset) / scale;
    }

    // Set gain: valid values 128, 64, 32
    void setGain(uint16_t g) {
        if (g == 128) { gain = 128; gainPulses = 1; }
        else if (g == 64) { gain = 64; gainPulses = 3; }
        else if (g == 32) { gain = 32; gainPulses = 2; }
        else { gain = 128; gainPulses = 1; } // default
    }
    uint16_t getGain() const { return gain; }

    // Power down the HX711 (pull PD_SCK high for > 60us)
    void powerDown() {
        digitalWrite(sckPin, LOW);
        digitalWrite(sckPin, HIGH);
        delayMicroseconds(70);
    }

    // Power up the HX711 (pull PD_SCK low)
    void powerUp() {
        digitalWrite(sckPin, LOW);
        delayMicroseconds(100);
    }

private:
    uint8_t doutPin;
    uint8_t sckPin;
    uint16_t gain;
    uint8_t gainPulses; // 1,2 or 3 pulses after 24 data clocks
    float scale;
    long offset;
};