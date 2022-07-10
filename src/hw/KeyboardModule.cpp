#include "Tasker.h"
#include "KeyboardModule.h"

#define KEY_PAUSE_THRESHOLD 30

char keyboardKeys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
};

KeyboardModule::KeyboardModule() {
    this->pcf = new PCF8574(I2C_ADDR_KEYBOARD);
}

bool KeyboardModule::begin() {
    clean();

    {
        std::lock_guard<std::mutex> lg(HwLocks::I2C);
        Wire.setClock(I2C_FREQ_PCF8574);
        if (!pcf->begin()) {
            Debug.println("Couldn't initialize PCF module!");
            return false;
        }
    }

    Core0.loopEvery("handleKeyPress", 10, [this] {
        handleKeyPress();
    });

    return true;
}

void KeyboardModule::handleKeyPress() {
    char current = keyboardRead();
    char wasPressed = 0;

    if (current != lastKey && millis() - lastKeyTime < KEY_PAUSE_THRESHOLD) {
        // Skipping current key - may be shimmering
        return;
    }

    if (current != 0) {
        // pressed key!
        lastKey = current;
        lastKeyTime = millis();
    } else {
        // released or nothing
        if (lastKey != 0) {
            wasPressed = lastKey;
            lastKey = 0;
        }
    }

    if (wasPressed == 0) return;

    switch (wasPressed) {
        case '*':
            if (KEYS_LOGGING) Debug.println("Cleaning keyboard buffer");
            for (auto &callback: codeEnteredCallbacks) callback(KEYBOARD_CLEAN);
            clean();
            break;
        case '#':
            if (KEYS_LOGGING) Debug.printf("Pressed #, result code: %s\n", pressedKeyBuffer.c_str());
            for (auto &callback: codeEnteredCallbacks) callback(pressedKeyBuffer);
            clean();
            break;
        default:
            if (KEYS_LOGGING) Debug.printf("Pressed key: %c\n", wasPressed);

            append(wasPressed);
    }
}

void KeyboardModule::clean() {
    this->pressedKeyBuffer = "";
}

void KeyboardModule::append(char c) {
    this->pressedKeyBuffer.concat(c);
}

char KeyboardModule::keyboardRead() {
    char key = 0;

    std::lock_guard<std::mutex> lg(HwLocks::I2C);
    Wire.setClock(I2C_FREQ_PCF8574);

    for (int row = 0; row <= 3; row++) {
        int col = 0;

        pcf->write8(0xff ^ (1 << row));

        const auto value = (pcf->read8() & 0xf0) >> 4;
        for (; col <= 3; col++) {
            if (value + (1 << col) == 0xf) break;
        }

        if (col > 3) continue;

        key = keyboardKeys[row][col];
    }

    return key;
}

void KeyboardModule::addCallback(const std::function<void(String)> &callback) {
    codeEnteredCallbacks.push_back(callback);
}
