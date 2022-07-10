#include <Arduino.h>

#include "Tasker.h"
#include "defs.h"

#include "PortalFramework.h"
#include "hw/LedRing.h"
#include "hw/Display.h"
#include "hw/KeyboardModule.h"
#include "StateManager.h"
#include "Portal.h"

StateManager stateManager;
KeyboardModule keyboardModule;
PortalFramework framework;
LedRing ledRing;
Display display;
Portal portal;

void setup() {
    Serial.begin(115200);
    // wait for monitor open
    delay(500);

    std::optional<std::string> frameworkInitMessage = framework.begin(false, true);

    ledRing.begin();

    if (!portal.begin(&framework, &keyboardModule, &ledRing)) {
        Debug.println("Could not initialize portal!");
        return;
    }

    stateManager.begin(&portal, &framework, &keyboardModule);

    if (!display.begin(&stateManager)) {
        Debug.println("Could not initialize display!");
        return;
    }

    if (!frameworkInitMessage->empty()) {
        Debug.printf("Could not initialize framework! Err: %s\n", frameworkInitMessage.value().c_str());
        const String &err = String(frameworkInitMessage.value().c_str());
        portal.showErrorMessage(&err);
    }

    if (!keyboardModule.begin()) {
        const String msg = String("Could not initialize keyboard!");
        Debug.println(msg);
        portal.showErrorMessage(&msg);
        return;
    }

    ledRing.makeSpiral(LEDRING_COLOR_GREEN);
    Debug.println("Setup finished");
    Debug.println("------------------------------------\n");
}

void loop() {
    // no-op - ESP32
}
