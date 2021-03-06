#ifndef PORTAL_CONSTANTS_H
#define PORTAL_CONSTANTS_H

#include "Adafruit_NeoPixel.h"
#include "Arduino.h"
#include "FS.h"
#include "SPIFFS.h"
#include <map>

#include "debugging.h"
#include "Tasker.h"
#include "HwLocks.h"
#include "types.h"

#define DNS_PORT 53

#define LEDRING_COLOR_RED Adafruit_NeoPixel::Color(150, 0, 0)
#define LEDRING_COLOR_GREEN Adafruit_NeoPixel::Color(0, 150, 0)
#define LEDRING_COLOR_BLUE Adafruit_NeoPixel::Color(0, 0, 150)
#define LEDRING_COLOR_ORANGE Adafruit_NeoPixel::Color(255,165,0)

#define LEDRING_BRIGHTNESS 5
#define LEDRING_BITS 16
#define LEDRING_BLINK_LENGTH 200
#define LEDRING_SPIRAL_LENGTH 400

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DISPLAY_ERROR_TIMEOUT 15000
#define DISPLAY_WARN_TIMEOUT 5000
#define DISPLAY_INFO_TIMEOUT 2000

#define I2C_ADDR_KEYBOARD 0x20

#define I2C_FREQ_RTC 100000
#define I2C_FREQ_PCF8574 100000

#define PIN_LED_RING 14
#define PIN_MODE 4
#define PIN_BUTTON_INIT_TAG 0
#define PIN_DISPLAY_RESET 33

const String helpSelected = "(# - potvrdit)\n(* - zrusit zadani)";

const String RECOVERY_CODE = "C9";
const String SYNC_CODE = "C1";

#endif //PORTAL_CONSTANTS_H
