#ifndef PORTAL_DISPLAY_H
#define PORTAL_DISPLAY_H

#include <TFT_eSPI.h>
#include <StateManager.h>

#include "Constants.h"

#define TFT_GREY 0x5AEB // New colour


class Display {
public:
    bool begin(StateManager *stateManager);

    TFT_eSPI tft;
private:
    bool waitingForRedraw = false;

    void draw(AppState state, std::optional<ModalMessage> modalMessage);

    u64 displayingModalUntil = 0;

    StateManager *stateManager;
};


#endif //PORTAL_DISPLAY_H
