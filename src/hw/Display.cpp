#include <Wire.h>
#include "Display.h"
#include "HwLocks.h"

bool Display::begin(StateManager *stateManager) {
    this->stateManager = stateManager;

    pinMode(PIN_DISPLAY_RESET, OUTPUT);

    {
        // this resets the display
        digitalWrite(PIN_DISPLAY_RESET, LOW);
        Tasker::sleep(50);
        digitalWrite(PIN_DISPLAY_RESET, HIGH);
        Tasker::sleep(50);
        digitalWrite(PIN_DISPLAY_RESET, LOW);
        Tasker::sleep(50);
        digitalWrite(PIN_DISPLAY_RESET, HIGH);
        Tasker::sleep(50);

        // now initialize
        std::lock_guard<std::mutex> lg(HwLocks::SPI);
        tft.init();
        tft.setRotation(1);

        tft.fillScreen(TFT_GREY);
        tft.fillRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 10, TFT_RED);

        Debug.println("Display initialized");
    }

    this->stateManager->addCallback([this](const AppState state) {
        waitingForRedraw = true;
    });

    Core1.loopEvery("Display", 50, [this] {
        // redraw when asked, after modal was shown ("hide it") or when new modal is ready
        if (waitingForRedraw || (displayingModalUntil > 0 && millis() > displayingModalUntil) || this->stateManager->hasModalMessage()) {
            const AppState &appState = this->stateManager->getState();
            const std::optional<ModalMessage> modalMessage = this->stateManager->popModalMessage();

            if (appState.mode == PortalMode::Starting) return;

            draw(appState, modalMessage);
        }
    });

    return true;
}

void Display::draw(const AppState state, const std::optional<ModalMessage> modalMessage) {
    waitingForRedraw = false;

    // don't redraw if modal is displayed
    if (millis() <= displayingModalUntil) return;
    displayingModalUntil = 0;

    if (modalMessage.has_value()) {
        auto mm = modalMessage.value();

        const auto bgColor = mm.modalMessageType == Info ? TFT_GREEN : TFT_RED;
        const auto fgColor = mm.modalMessageType == Info ? TFT_BLACK : TFT_WHITE;

        std::lock_guard<std::mutex> lg(HwLocks::SPI);

        tft.fillRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 5, bgColor);
        tft.setCursor(0, 0, 2);
        tft.setTextColor(fgColor);
        tft.setTextSize(2);
        tft.println(mm.text);

        displayingModalUntil = millis() + (mm.modalMessageType == Info ? DISPLAY_INFO_TIMEOUT : DISPLAY_ERROR_TIMEOUT);

        return;
    }

    String playerDataStr = "";

    if (state.tagPresent) {
        playerDataStr += "Hrac ";
        playerDataStr += String(state.currentPlayerData.user_id);
        playerDataStr += ", sila ";
        playerDataStr += String(state.currentPlayerData.strength);
    } else {
        playerDataStr += "Neni vlozen tag!";
    }

    const String itemSelected = state.stage == PortalStage::Stage2 ? "Vybrano: " + state.itemToConfirm : "";

    // -----------

    const auto bgColor = state.tagPresent ? (state.mode == PortalMode::User ? TFT_GREEN : TFT_ORANGE) : TFT_RED;
    const auto fgColor = state.tagPresent ? TFT_BLACK : TFT_WHITE;

    std::lock_guard<std::mutex> lg(HwLocks::SPI);

    tft.fillRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 5, bgColor);
    tft.setCursor(0, 0, 2);
    tft.setTextColor(fgColor);
    tft.setTextSize(1);
    tft.println(playerDataStr);
    tft.println();
    tft.println(itemSelected);
}
