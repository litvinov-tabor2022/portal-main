#include <Arduino.h>

#include "UpdateOTA.h"

#include "Tasker.h"
#include "defs.h"

#include "networking/WebServer.h"
#include "PortalFramework.h"
#include "hw/LedRing.h"
#include "hw/Display.h"
#include "hw/KeyboardModule.h"
#include "StateManager.h"
#include "Portal.h"

StateManager stateManager;
KeyboardModule keyboardModule;
PortalFramework framework;
WebServer webServer(&framework);
LedRing ledRing;
Display display;
Portal portal;
Storage storage;

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

    stateManager.begin(&portal);

    if (!display.begin(&stateManager)) {
        Debug.println("Could not initialize display!");
        return;
    }

    if (!keyboardModule.begin()) {
        Debug.println("Could not initialize keyboard!");
        return;
    }

    Core0.once("AP start", [] {
        // TODO load this from EEPROM
        String ssid = "portal-123";
        String password = "12345678";
        Debug.println("Starting AP...");
        AccessPoint::start(ssid.c_str(), password.c_str());
        Debug.println("Starting OTA...");
        ArduinoOTA.begin();
        TelnetPrint.begin();
        // TODO keep this for testing
        Debug.println(F("Starting HTTP server..."));
        webServer.start();
    });

    setupOTA();

    Core0.loop("OTA", [] {
        ArduinoOTA.handle();
    });

    stateManager.addCallback([](const AppState state) {
        // TODO in prod, it will switch HTTP server here

        switch (state.mode) {
            case PortalMode::User:
                keyboardModule.setReadingEnabled(state.tagPresent);
                break;
            case PortalMode::Service:
                keyboardModule.setReadingEnabled(false);
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
