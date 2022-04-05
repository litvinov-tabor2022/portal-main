#ifndef PORTAL_TYPES_H
#define PORTAL_TYPES_H

#include <Arduino.h>
#include <PortalFramework.h>

enum PortalStage {
    Stage0, Stage1, Stage2, Stage3, Stage4
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
    Default, Skills
};

struct PriceListEntry {
    String code;
    String name;
    i16 strength;
    i16 magic;
    i16 dexterity;
    i16 skill;
    i16 bonus_points;
};

#endif //PORTAL_TYPES_H
