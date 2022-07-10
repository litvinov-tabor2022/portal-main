#ifndef PORTAL_DISPLAY_H
#define PORTAL_DISPLAY_H

#include <TFT_eSPI.h>
#include <StateManager.h>

#include "Constants.h"

#define TFT_GREY 0x5AEB // New colour


class Display {
public:
    bool begin(StateManager *stateManager, Portal *portal);

    TFT_eSPI tft;
private:
    static String selectedItemToString(const PriceListEntry &item);

    bool waitingForRedraw = false;

    void draw(AppState state, std::optional<ModalMessage> modalMessage);

    u64 displayingModalUntil = 0;

    StateManager *stateManager;
    Portal *portal;
};


#endif //PORTAL_DISPLAY_H
