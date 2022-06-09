#include <set>
#include "Portal.h"
#include "utils.h"
#include "PlayerDataUtils.h"

void Portal::begin(PortalFramework *pFramework, KeyboardModule *keyboard, LedRing *ledRing) {
    this->framework = pFramework;
    this->priceList = framework->resources.loadPriceList();
    this->ledRing = ledRing;
    this->keyboardModule = keyboard;

    this->framework->addOnConnectCallback([this](PlayerData playerData, bool isReload) {
        handleConnectedTag(playerData);
    });

    this->framework->addOnDisconnectCallback([this]() {
        flagTagDisconnected = true;
        onPlayerTagDisconnected();
    });

    keyboardModule->addCallback([this](String code) {
        lastCodeEntered = std::move(code);
        flagCodeEntered = true;
    });

    Core0.loopEvery("InitializeTag", 20, [this] {
        if (lastCodeEntered == RECOVERY_CODE && this->framework->isTagConnected()) {
            Serial.println("Recovering to last inserted tag");
            if (recoverTag()) {
                this->ledRing->blink(LEDRING_COLOR_GREEN);
            } else {
                this->ledRing->blink(LEDRING_COLOR_RED);
            }
            lastCodeEntered = "";
        }

        if (digitalRead(PIN_BUTTON_INIT_TAG) == LOW) {
            if (this->framework->initializeTag()) {
                this->ledRing->blink(LEDRING_COLOR_GREEN);
            } else {
                this->ledRing->blink(LEDRING_COLOR_RED);
            }
        }
    });

    Debug.println("Portal started!");
}

void Portal::handleConnectedTag(PlayerData playerData) {
    if (framework->isTagConnected()) {
        keyboardModule->clean();
        String strSkills = "";
        for (auto skill: playerData.skills) {
            strSkills += static_cast<int>(skill);
            strSkills += " ";
        }
        Debug.printf("Connected player: ID %d, strength=%d magic=%d dexterity=%d skills=%s bonus_points=%d\n",
                     playerData.user_id, playerData.strength, playerData.magic, playerData.dexterity,
                     strSkills.c_str(), playerData.bonus_points);

        onPlayerTagConnected(playerData);
        ledRing->makeSpiral(LEDRING_COLOR_GREEN);

        while (framework->isTagConnected()) {
            stage1();
            if (framework->isTagConnected()) Debug.println("Tag handling done, restart the flow");
        }

        Debug.println("Tag handling done, quit");
        onStageChange(PortalStage::Stage0);
        resetFlags();
    } else {
        Debug.println("Tag disconnected during reading, quit");
        resetFlags();
        return;
    }
}

PortalEvent Portal::waitForEvent() {
    if (!framework->isTagConnected()) {
        return TagDisconnected;
    }

    resetFlags();

    while (!flagCodeEntered && !flagTagDisconnected) {
        Tasker::sleep(50); // busy wait

        if (!framework->isTagConnected()) {
            return TagDisconnected;
        }
    }

    if (flagTagDisconnected) return TagDisconnected;
    //else
    return CodeEntered;
}

void Portal::stage1() {
    while (framework->isTagConnected()) {
        onStageChange(PortalStage::Stage1);

        switch (waitForEvent()) {
            case TagDisconnected:
                Debug.println("Tag disconnected, aborting");
                Debug.println("--------------------------");
                return;

            case CodeEntered:
                const String code = lastCodeEntered;
                if (code == "" || code == KEYBOARD_CLEAN) {
                    String message = "Zadavani zruseno";
                    onInfoMessage(&message);
                    break; // break switch -> repeat
                }

                const std::optional<PriceListEntry> entry = priceList->getItemForCode(code);
                if (entry.has_value()) { // found!
                    Debug.printf(
                            "Pricelist item found: altName=%s strength=%d magic=%d dexterity=%d skill=%d\n",
                            entry->altName.c_str(), entry->constraints.strength, entry->constraints.magic,
                            entry->constraints.dexterity, entry->skill);

                    onItemSelected(entry.value());

                    if (stage2(&entry.value())) {
                        return; // everything done!
                    }
                    Debug.println("Order was NOT confirmed; repeat input");
                    //else
                    break; // break switch -> repeat
                } else {
                    Debug.printf("PriceList entry for '%s' not found; repeat input\n", code.c_str());
                    String message = "Polozka s kodem \n    " + code + "\nnenalezena";
                    onInfoMessage(&message);
                    break; // break switch -> repeat
                }
        }
    }

    // in theory, it should never reach here...
    Debug.println("Tag disconnected, aborting (it should have never reached here... did some error occur?)");
    Debug.println("--------------------------");
}

bool Portal::stage2(const PriceListEntry *entry) {
    onStageChange(PortalStage::Stage3);

    if (!PlayerDataUtils::canHaveSkill(*entry, currentPlayerData) && entry->operation == ADD) {
        String s = "";
        if (entry->constraints.magic > currentPlayerData.magic) {
            s += "\nmagie: " + (String) currentPlayerData.magic + "/" + entry->constraints.magic;
        }
        if (entry->constraints.dexterity > currentPlayerData.dexterity) {
            s += "\nobratnost: " + (String) currentPlayerData.dexterity + "/" + entry->constraints.dexterity;
        }
        if (entry->constraints.strength > currentPlayerData.strength) {
            s += "\nsila: " + (String) currentPlayerData.strength + "/" + (String) entry->constraints.strength;
        }
        String message = "Nedostatecna uroven\nvlastnosti pro schopnost\n---------------\n" + entry->altName;
        message += s;
        onInfoMessage(&message);
        return false;
    }

    stage3(entry);
    return true;
}

bool Portal::stage3(const PriceListEntry *entry) {
    onStageChange(PortalStage::Stage2);

    switch (waitForEvent()) {
        case TagDisconnected:
            Debug.println("Tag disconnected, aborting");
            Debug.println("--------------------------");
            return true;

        case CodeEntered:
            if (lastCodeEntered == KEYBOARD_CLEAN) {
                Debug.println("Entered item canceled");
                return false;
            }

            if (lastCodeEntered == "") {
                // empty code is a confirmation!
                return stage4(entry);
            }
            // else

            return false;
    }

    Debug.println("It should never reach here! 11");
    return true;
}

bool Portal::stage4(const PriceListEntry *entry) {
    onStageChange(PortalStage::Stage4);

    Debug.printf("Charging item: name=%s strength=%d magic=%d dexterity=%d skill=%d\n",
                 entry->altName.c_str(), entry->constraints.strength, entry->constraints.magic,
                 entry->constraints.dexterity, entry->skill);

    const Transaction t = Transaction{
            .time = framework->clocks.getCurrentTime(),
            .device_id = framework->getDeviceConfig().deviceId,
            .user_id= static_cast<u8>(currentPlayerData.user_id),
            .skill = static_cast<i16>(static_cast<i16>(entry->skill) * (entry->operation == ADD ? 1 : -1))
    };

    if (!framework->storage.appendTransaction(t)) {
        Debug.println("Can't write to the commit log!");
        return false;
    }

    switch (entry->operation) {
        case ADD:
            Debug.printf("Inserting skill: %d\n", entry->skill);
            PlayerDataUtils::addSkill(entry->skill, &currentPlayerData);
            break;
        case REMOVE: {
            Debug.printf("Deleting skill: %d\n", currentPlayerData.skills_count);
            PlayerDataUtils::removeSkill(entry->skill, &currentPlayerData);
            break;
        }
    }

    if (!this->framework->writePlayerData(currentPlayerData)) {
        Debug.println("Could not charge selected item!");
        Core0.once("blink", [this] {
            ledRing->blink(LEDRING_COLOR_RED);
        });
        return false;
    }

    Core0.once("blink", [this] {
        ledRing->blink(LEDRING_COLOR_GREEN);
    });

    return true;
}

bool Portal::recoverTag() {
    return this->framework->writePlayerData(currentPlayerData);
}
