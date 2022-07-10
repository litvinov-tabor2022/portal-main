#include <Wire.h>
#include "Display.h"
#include "HwLocks.h"

bool Display::begin(StateManager *stateManager, Portal *portal) {
    this->stateManager = stateManager;
    this->portal = portal;

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

    this->stateManager->addCallback([this](const AppState &state) {
        waitingForRedraw = true;
    });

    Core1.loopEvery("Display", 50, [this] {
        // redraw when asked, after modal was shown ("hide it") or when new modal is ready
        if (waitingForRedraw || (displayingModalUntil > 0 && millis() > displayingModalUntil) ||
            this->stateManager->hasModalMessage()) {
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

        const auto bgColor = mm.modalMessageType == Info ? TFT_ORANGE : TFT_RED;
        const auto fgColor = mm.modalMessageType == Info ? TFT_BLACK : TFT_WHITE;
        const auto fontSize = mm.modalMessageType == Info ? 1 : 2;
        std::lock_guard<std::mutex> lg(HwLocks::SPI);

        tft.fillRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 5, bgColor);
        tft.setCursor(0, 0);
        tft.setTextColor(fgColor);
        tft.setTextSize(fontSize);
        tft.println(mm.text);

        displayingModalUntil = millis() + mm.duration;

        return;
    }

    String playerDataStr = "";

    if (state.mode == PortalMode::Switching) {
        playerDataStr += "\nCekej!!!";
    } else {
        if (state.tagPresent) {
            playerDataStr += "Hrac ";
            playerDataStr += String(state.currentPlayerData.user_id);
            playerDataStr += " (";
            playerDataStr += portal->getPlayerMetadata(state.currentPlayerData.user_id).name.c_str();
            playerDataStr += ")\nsila.................";
            playerDataStr += String(state.currentPlayerData.strength);
            playerDataStr += "\nobratnost..........";
            playerDataStr += String(state.currentPlayerData.dexterity);
            playerDataStr += "\nmagie...............";
            playerDataStr += String(state.currentPlayerData.magic);
            playerDataStr += "\nvolne zkusenosti..";
            playerDataStr += String(state.currentPlayerData.bonus_points);
            playerDataStr += "\npocet schopnosti..";
            playerDataStr += String(state.currentPlayerData.skills_count);
        } else {
            playerDataStr += "Neni vlozen amulet!";
            if (state.mode == PortalMode::Service) {
                playerDataStr += "\nSERVISNI MOD, ID:\n\n";
                playerDataStr += portal->framework->getDeviceConfig().deviceId;
            }
        }
    }

    const String itemSelected =
            state.stage == PortalStage::Stage2 ? "Vybrano:\n --------- \n" + selectedItemToString(state.itemToConfirm) : "";

    // -----------

    const auto bgColor = state.tagPresent ? (state.mode == PortalMode::User ? TFT_GREEN : TFT_ORANGE) : TFT_RED;
    const auto fgColor = state.tagPresent ? TFT_BLACK : TFT_WHITE;

    std::lock_guard<std::mutex> lg(HwLocks::SPI);

    tft.fillRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 5, bgColor);
    tft.setCursor(0, 0, 2);
    tft.setTextColor(fgColor);
    tft.setTextSize(1);
    tft.setTextWrap(true, true);

    if (state.stage == PortalStage::Stage2) {
        tft.println(itemSelected);
        tft.println();
        tft.println(helpSelected);
    } else {
        tft.println(playerDataStr);
    }
}

String Display::selectedItemToString(const PriceListEntry &item) {
    String res = "";

    switch (item.operation) {
        case ADD:
            res += "Pridej:\n";
            break;
        case REMOVE:
            res += "Odeber:\n";
        default:
            break;
    }

    res += item.altName;
    return res;
}
