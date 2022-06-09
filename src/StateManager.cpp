#include "StateManager.h"

#include "Tasker.h"
#include "Constants.h"

void StateManager::begin(Portal *portal, PortalFramework *framework, KeyboardModule *keyboard) {
    pinMode(PIN_MODE, INPUT_PULLUP);

    this->state.mode = User;
    keyboard->addCallback([this, framework] (const String& code) mutable  {
        if(code == SYNC_CODE){
            Serial.println("Entering sync mode");
            framework->synchronizationMode.toggle();
            const PortalMode oldMode = this->state.mode;
            const PortalMode newMode = oldMode == User ? Service : User;

            this->state.mode = newMode;
            onStateChange();
        }
    });

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
