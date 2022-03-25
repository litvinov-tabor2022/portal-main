#ifndef PORTAL_TYPES_H
#define PORTAL_TYPES_H

#include <Arduino.h>
#include <PortalFramework.h>

enum PortalStage {
    Stage0, Stage1, Stage2, Stage3
};

enum PortalMode {
    Starting, User, Service
};

enum ModalMessageType {
    Info, Error
};

struct AppState {
    PortalMode mode;
    bool tagPresent;
    PortalStage stage;
    String itemToConfirm;
    PlayerData currentPlayerData; // this is valid only when tagPresent == true!
};

struct ModalMessage {
    String text;
    ModalMessageType modalMessageType;
};

enum PortalRole {
    Default
};

struct PriceListEntry {
    String code;
    String name;
    i8 price;
    i8 delta;
};

#endif //PORTAL_TYPES_H
