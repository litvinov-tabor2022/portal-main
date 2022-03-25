#include "Arduino.h"

#include "Constants.h"
#include "LedRing.h"

void LedRing::begin() {
    std::lock_guard<std::mutex> lg(mutex);
    pixels = new Adafruit_NeoPixel(LEDRING_BITS, PIN_LED_RING, NEO_GRB + NEO_KHZ800);

    pixels->begin();
    pixels->setBrightness(LEDRING_BRIGHTNESS);
    pixels->clear();
    pixels->show();
}

void LedRing::makeSpiral(u32 color) {
    const u8 sleep = (LEDRING_SPIRAL_LENGTH - 50) / LEDRING_BITS;

    for (int i = 0; i < LEDRING_BITS; i++) {
        pixels->setPixelColor(i, color);
        pixels->show();
        Tasker::sleep(sleep);
    }

    Tasker::sleep(50);
    pixels->clear();
    pixels->show();
}

void LedRing::blink(u32 color) {
    std::lock_guard<std::mutex> lg(mutex);

    // save current state
    uint32_t old[LEDRING_BITS];
    for (int i = 0; i < LEDRING_BITS; i++) {
        old[i] = pixels->getPixelColor(i);
    }

    pixels->clear();
    pixels->show();
    pixels->fill(color, 0, LEDRING_BITS);
    pixels->show();
    Tasker::sleep(LEDRING_BLINK_LENGTH);

    // restore original state
    for (int i = 0; i < LEDRING_BITS; i++) {
        pixels->setPixelColor(i, old[i]);
    }
    pixels->show();
}

void LedRing::set(u8 pin, u32 color) {
    std::lock_guard<std::mutex> lg(mutex);

    pixels->setPixelColor(pin, color);
    pixels->show();
}
