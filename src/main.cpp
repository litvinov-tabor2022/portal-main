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

    std::optional<std::string> frameworkInitMessage = framework.begin();
    if (!frameworkInitMessage->empty()) {
        Debug.printf("Could not initialize framework! Err: %s\n", frameworkInitMessage.value().c_str());
        return;
    }

    ledRing.begin();

    portal.begin(&framework, &keyboardModule, &ledRing);

    stateManager.begin(&portal, &framework, &keyboardModule);

    if (!display.begin(&stateManager)) {
        Debug.println("Could not initialize display!");
        return;
    }

    if (!keyboardModule.begin()) {
        Debug.println("Could not initialize keyboard!");
        return;
    }

    stateManager.addCallback([](const AppState state) {
        // TODO in prod, it will switch HTTP server here

        switch (state.mode) {
            case PortalMode::User:
//                keyboardModule.setReadingEnabled(state.tagPresent);
                keyboardModule.setReadingEnabled(true);
                break;
            case PortalMode::Service:
                ledRing.makeSpiral(LEDRING_COLOR_ORANGE);
                keyboardModule.setReadingEnabled(true);
                break;
            case PortalMode::Starting:
                // no-op
                break;
        }
    });

    ledRing.makeSpiral(LEDRING_COLOR_GREEN);
    Debug.println("Setup finished");
    Debug.println("------------------------------------\n");
}

void loop() {
    // no-op - ESP32
}
