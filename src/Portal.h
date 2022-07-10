#ifndef PORTAL_PORTAL_H
#define PORTAL_PORTAL_H

#include "PortalFramework.h"
#include "hw/LedRing.h"
#include "hw/KeyboardModule.h"
#include "defs.h"
#include "Resources.h"

enum PortalEvent {
    TagDisconnected, CodeEntered
};

class Portal {
public:
    bool begin(PortalFramework *pFramework, KeyboardModule *keyboard, LedRing *ledRing);

    void addStageChangeCallback(const std::function<void(PortalStage)> &callback) { stageChangeCallbacks.push_back(callback); }

    void addItemSelectedCallback(const std::function<void(const PriceListEntry &)> &itemName) { itemSelectedCallbacks.push_back(itemName); }

    void addPlayerConnectCallback(const std::function<void(_portal_PlayerData)> &callback) {
        playerTagConnectedCallbacks.push_back(callback);
    }

    void addPlayerDisconnectCallback(const std::function<void()> &callback) { playerTagDisconnectedCallbacks.push_back(callback); }

    void addErrorCallback(const std::function<void(const String *)> &callback) { errorCallbacks.push_back(callback); }

    void addWarnCallback(const std::function<void(const String *)> &callback) { warnCallbacks.push_back(callback); }

    void addInfoCallback(const std::function<void(const String *, int duration)> &callback) { infoCallbacks.push_back(callback); }

    void showErrorMessage(const String *text) {
        onErrorMessage(text);
    }

    void showWarnMessage(const String *text) {
        onWarnMessage(text);
    }

    void showInfoMessage(const String *text, int duration = DISPLAY_INFO_TIMEOUT) {
        onInfoMessage(text, duration);
    }

    [[nodiscard]] PlayerMetadata getPlayerMetadata(u8 userId) const;

    PortalFramework *framework;
    LedRing *ledRing;
private:
    void handleConnectedTag(PlayerData playerData);

    PortalEvent waitForEvent();

    void onStageChange(const PortalStage stage) {
        for (auto &callback: stageChangeCallbacks) callback(stage);
    }

    void onItemSelected(const PriceListEntry &item) {
        for (auto &callback: itemSelectedCallbacks) callback(item);
    }

    void onPlayerTagConnected(const PlayerData playerData) {
        this->currentPlayerData = playerData;
        for (auto &callback: playerTagConnectedCallbacks) callback(playerData);
    }

    void onPlayerTagDisconnected() {
        for (auto &callback: playerTagDisconnectedCallbacks) callback();
    }

    void onErrorMessage(const String *text) {
        for (auto &callback: errorCallbacks) callback(text);
    }

    void onWarnMessage(const String *text) {
        for (auto &callback: warnCallbacks) callback(text);
    }

    void onInfoMessage(const String *text, int duration = DISPLAY_INFO_TIMEOUT) {
        for (auto &callback: infoCallbacks) callback(text, duration);
    }

    void stage1();

    bool stage2(const PriceListEntry *entry);

    bool stage3(const PriceListEntry *entry);

    bool stage4(const PriceListEntry *entry);

    void resetFlags() {
        flagTagDisconnected = false;
        flagCodeEntered = false;
    }

    bool recoverTag();

    PriceList *priceList;
    KeyboardModule *keyboardModule;

    bool flagCodeEntered = false;
    bool flagTagDisconnected = false;
    PlayerData currentPlayerData = portal_PlayerData_init_zero;
    PlayersMetadata *playersMetadata;

    String lastCodeEntered = "";

    std::vector<std::function<void(const PriceListEntry &)>> itemSelectedCallbacks;
    std::vector<std::function<void(PortalStage)>> stageChangeCallbacks;
    std::vector<std::function<void(_portal_PlayerData playerData)>> playerTagConnectedCallbacks;
    std::vector<std::function<void()>> playerTagDisconnectedCallbacks;
    std::vector<std::function<void(const String *)>> errorCallbacks;
    std::vector<std::function<void(const String *)>> warnCallbacks;
    std::vector<std::function<void(const String *, int duration)>> infoCallbacks;
};


#endif //PORTAL_PORTAL_H
