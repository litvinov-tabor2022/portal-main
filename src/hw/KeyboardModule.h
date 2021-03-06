#ifndef PORTAL_KEYBOARDMODULE_H
#define PORTAL_KEYBOARDMODULE_H

#include "PCF8574.h"

#include "Constants.h"

#define KEYS_LOGGING false

#define KEYBOARD_CLEAN "__CLEAN__"

class KeyboardModule {
public:
    explicit KeyboardModule();

    bool begin();

    void addCallback(const std::function<void(String)> &callback);

    void clean();

private:
    char keyboardRead();

    void append(char c);

    void handleKeyPress();

    PCF8574 *pcf;

    String pressedKeyBuffer;
    char lastKey = 0;
    unsigned long lastKeyTime = 0;
    std::vector<std::function<void(String)>> codeEnteredCallbacks;
};

#endif //PORTAL_KEYBOARDMODULE_H
