#include <Arduino.h>
#include <TimerOne.h>

const uint8_t audioOutPin = 9;
const uint8_t LEDPin = 5;

const uint8_t octaveUpPin = 2;
const uint8_t octaveDownPin = 3;

const uint8_t freqAdjPin = A0;

const int readInterval = 1; // millis

const unsigned int cvInBits = 10;
const unsigned int cvInMaxVal = (1 << cvInBits) - 1;

const double minCV = 0.0; // min 0V (clamp with diodes etc)
const double maxCV = 5.0; // max 5V

const double minFreq = 27.5; // freq for CV input 0 - this equates to 0V (this can be overridden with octave switch)
const double maxFreq = pow(2, maxCV) * minFreq;

unsigned long currentHalfPeriod = 0;

int outputState = LOW;

inline void toggleOutputState() {
    if (outputState == LOW) {
        outputState = HIGH;
    } else {
        outputState = LOW;
    }
    digitalWrite(audioOutPin, outputState);
}

inline unsigned long getHalfPeriod(double freq) {
    return lround(500000.0/ freq);
}

double getFreq() {
    double freqCVValue = (double) analogRead(freqAdjPin) / cvInMaxVal * (maxCV - minCV) + minCV; // read freq pot and scale to CV

    double freq = pow(2, freqCVValue) * minFreq;

    // adjust octaves
    if (digitalRead(octaveUpPin) == HIGH) freq = freq * 2;
    if (digitalRead(octaveDownPin) == HIGH) freq = freq / 2;

    return freq;
}

__attribute__((unused)) void setup(void) {
    pinMode(audioOutPin, OUTPUT);
    pinMode(LEDPin, OUTPUT);

    pinMode(freqAdjPin, INPUT);
    pinMode(octaveDownPin, INPUT);
    pinMode(octaveUpPin, INPUT);

    // get initial reading from pot
    double freq = getFreq();
    currentHalfPeriod = getHalfPeriod(freq);

    Timer1.initialize(currentHalfPeriod);
    Timer1.attachInterrupt(toggleOutputState);
}

__attribute__((unused)) void loop(void) {
    delay(readInterval);
    double freq = getFreq();

    unsigned long newHalfPeriod = getHalfPeriod(freq);

    currentHalfPeriod = newHalfPeriod;
    Timer1.setPeriod(newHalfPeriod);

    const int LEDBrightness = (int) lround((exp((double) freq / ((double) maxFreq * 2)) - 1) / M_E * 255); // *2 as we need to account for +1 octave
    analogWrite(LEDPin, LEDBrightness);
}