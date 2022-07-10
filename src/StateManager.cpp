#include "StateManager.h"

#include "Tasker.h"
#include "Constants.h"

void StateManager::begin(Portal *portal, PortalFramework *framework, KeyboardModule *keyboard) {
    this->portal = portal;

    pinMode(PIN_MODE, INPUT_PULLUP);

    this->state.mode = User;
    keyboard->addCallback([this, framework, portal](const String &code) mutable {
        if (code == SYNC_CODE) {
            toggleSyncMode();
        } else {
            if (code.charAt(0) == 'C') {
                const String msg = String("Invalid control code: " + code);
                Debug.println(msg);
                portal->showInfoMessage(&msg, 1000);
            }
        }
    });

    portal->framework->addOnConnectCallback([this](PlayerData playerData, bool isReload) {
        if (playerData.user_id == ADMIN_USER_ID) {
            toggleSyncMode();
            return;
        }
    });

    portal->addStageChangeCallback([this](PortalStage stage) mutable {
        this->state.stage = stage;
        onStateChange();
    });

    portal->addItemSelectedCallback([this](const PriceListEntry &item) mutable {
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

    portal->addInfoCallback([this](const String *message, int duration) mutable {
        const ModalMessage mm = ModalMessage{
                .text = *message,
                .modalMessageType = ModalMessageType::Info,
                .duration = duration
        };

        this->modalMessage = mm;

        onStateChange();
    });

    portal->addWarnCallback([this](const String *message) mutable {
        const ModalMessage mm = ModalMessage{
                .text = *message,
                .modalMessageType = ModalMessageType::Warn,
                .duration = DISPLAY_WARN_TIMEOUT
        };

        this->modalMessage = mm;
        onStateChange();
    });

    portal->addErrorCallback([this](const String *message) mutable {
        const ModalMessage mm = ModalMessage{
                .text = *message,
                .modalMessageType = ModalMessageType::Error,
                .duration = DISPLAY_ERROR_TIMEOUT
        };

        this->modalMessage = mm;
        onStateChange();
    });

    Core1.loopEvery("RegularReport", 5000, [this] {
        reportState();
    });
}

void StateManager::toggleSyncMode() {
    portal->ledRing->asyncBlink(LEDRING_COLOR_ORANGE);
    Debug.println("Toggling sync mode");
    state.mode = Switching;
    onStateChange();

    String msg = String("Cekej!!!");
    portal->showInfoMessage(&msg);

    if (!portal->framework->synchronizationMode.toggle()) {
        msg = String("Could not switch mode");
        Debug.println(msg);
        portal->showErrorMessage(&msg);
    }

    state.mode = portal->framework->synchronizationMode.isStarted() ? Service : User;
    onStateChange();
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
            state.mode == User ? "User" : state.mode == Service ? "Service" : "Switching",
            stageStr.c_str(),
            state.tagPresent ? "" : " NOT",
            playerDataStr.c_str()
    );
}
