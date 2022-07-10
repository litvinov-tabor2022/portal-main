#ifndef PORTAL_STATEMANAGER_H
#define PORTAL_STATEMANAGER_H

#include "functional"
#include <optional>
#include "Arduino.h"

#include "MFRCTagReader.h"
#include "Portal.h"

class StateManager {
public:
    void begin(Portal *portal, PortalFramework *framework, KeyboardModule *keyboard);

    AppState getState() { return state; }

    std::optional<ModalMessage> popModalMessage() {
        auto r = this->modalMessage;
        this->modalMessage = std::nullopt;
        return r;
    }

    bool hasModalMessage() { return this->modalMessage.has_value(); }

    void addCallback(const std::function<void(AppState)> &callback);

private:
    void onStateChange();

    void reportState();

    void toggleSyncMode();

    AppState state = AppState{.mode = Starting};
    std::optional<ModalMessage> modalMessage = std::nullopt;
    std::vector<std::function<void(AppState)>> stateChangeCallbacks;

    Portal *portal;
};


#endif //PORTAL_STATEMANAGER_H
