#include "StateManager.h"

#include "Tasker.h"
#include "Constants.h"

void StateManager::begin(Portal *portal) {
    pinMode(PIN_MODE, INPUT_PULLUP);

    portal->addStageChangeCallback([this](PortalStage stage) mutable {
        this->state.stage = stage;
        onStateChange();
    });

    portal->addItemSelectedCallback([this](const PriceListEntry& item) mutable {
        this->state.itemToConfirm = PriceListEntry(item);
        onStateChange();
    });

    portal->addPlayerConnectCallback([this](const _portal_PlayerData playerData) mutable {
        this->state.currentPlayerData = playerData;
        this->state.tagPresent = true;
        onStateChange();
    });

    portal->addPlayerDisconnectCallback([this]() mutable {
        this->state.tagPresent = false;
        onStateChange();
    });

    portal->addInfoCallback([this](const String *message) mutable {
        const ModalMessage mm = ModalMessage{
                .text = *message,
                .modalMessageType = ModalMessageType::Info
        };

        this->modalMessage = mm;

        onStateChange();
    });

    portal->addErrorCallback([this](const String *message) mutable {
        const ModalMessage mm = ModalMessage{
                .text = *message,
                .modalMessageType = ModalMessageType::Error
        };

        this->modalMessage = mm;
        onStateChange();
    });

    Core1.loopEvery("ModeSwitchCheck", 100, [this]() mutable {
        const PortalMode oldMode = this->state.mode;
        const PortalMode newMode = digitalRead(PIN_MODE) == LOW ? Service : User;

        if (oldMode != newMode) {
            this->state.mode = newMode;
            onStateChange();
        }
    });

    Core1.loopEvery("RegularReport", 5000, [this] {
        reportState();
    });
}

void StateManager::onStateChange() {
    for (auto &callback: stateChangeCallbacks) callback(state);
}

void StateManager::addCallback(const std::function<void(AppState)> &callback) {
    stateChangeCallbacks.push_back(callback);
    callback(state);
}

void StateManager::reportState() {
    String stageStr;
    switch (state.stage) {
        case PortalStage::Stage0:
            stageStr = "-none-";
            break;
        case PortalStage::Stage1:
            stageStr = "Stage1";
            break;
        case PortalStage::Stage2:
            stageStr = "Stage2";
            break;
        case PortalStage::Stage3:
            stageStr = "Stage3";
            break;
        case PortalStage::Stage4:
            stageStr = "Stage4";
            break;
    }

    String playerDataStr = "";

    if (state.tagPresent) {
        playerDataStr += "; player ID ";
        playerDataStr += String(state.currentPlayerData.user_id);
        playerDataStr += ", str ";
        playerDataStr += String(state.currentPlayerData.strength);
    }

    Debug.printf(
            "STATE=[ mode %s; stage %s; tag%s inserted%s]\n",
            state.mode == User ? "User" : "Service",
            stageStr.c_str(),
            state.tagPresent ? "" : " NOT",
            playerDataStr.c_str()
    );
}
