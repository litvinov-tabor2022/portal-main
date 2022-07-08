#ifndef PORTAL_TYPES_H
#define PORTAL_TYPES_H

#include <Arduino.h>
#include <PortalFramework.h>
#include "Constants.h"

enum PortalStage {
    Stage0, Stage1, Stage2, Stage3, Stage4
};

enum PortalMode {
    Starting, User, Service
};

enum ModalMessageType {
    Info, Warn, Error
};

struct AppState {
    PortalMode mode;
    bool tagPresent;
    PortalStage stage;
    PriceListEntry itemToConfirm;
    PlayerData currentPlayerData; // this is valid only when tagPresent == true!
};

struct ModalMessage {
    String text;
    ModalMessageType modalMessageType;
    int duration;
};

#endif //PORTAL_TYPES_H
