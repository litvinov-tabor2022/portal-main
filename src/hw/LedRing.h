#ifndef PORTAL_LEDRING_H
#define PORTAL_LEDRING_H

#include "Adafruit_NeoPixel.h"
#include <mutex>
#include "types.h"

class LedRing {
public:
    void begin();

    void set(u8 pin, u32 color);

    void blink(u32 color);

    void makeSpiral(u32 color);

private:
    Adafruit_NeoPixel *pixels;
    std::mutex mutex;
};


#endif //PORTAL_LEDRING_H
