#include <set>
#include "Portal.h"
#include "utils.h"

void Portal::begin(PortalFramework *pFramework, KeyboardModule *keyboard, PriceList *priceList, LedRing *ledRing) {
    this->framework = pFramework;
    this->priceList = priceList;
    this->ledRing = ledRing;

    this->framework->addOnConnectCallback([this](PlayerData playerData) {
        handleConnectedTag(playerData);
    });

    this->framework->addOnDisconnectCallback([this]() {
        flagTagDisconnected = true;
        onPlayerTagDisconnected();
    });

    keyboard->addCallback([this](String code) {
        lastCodeEntered = std::move(code);
        flagCodeEntered = true;
    });

    Core0.loopEvery("InitializeTag", 20, [this] {
        if (!this->framework->isTagConnected()) return;

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

                const PriceListEntry *entry = priceList->getByCode(code);
                if (entry != nullptr) { // found!
                    Debug.printf(
                            "Pricelist item found: name=%s strength=%d magic=%d dexterity=%d skill=%d bonus_points=%d\n",
                            entry->name.c_str(), entry->strength, entry->magic, entry->dexterity, entry->skill,
                            entry->bonus_points);
                    onItemSelected(&entry->name);

                    if (stage2(entry)) {
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

    bool violateRestrictions = false;
    String s = "";

    if (entry->magic > currentPlayerData.magic) {
        s += "\nmagie: " + (String) currentPlayerData.magic + "/" + entry->magic;
        violateRestrictions = true;
    }
    if (entry->dexterity > currentPlayerData.dexterity) {
        s += "\nobratnost: " + (String) currentPlayerData.dexterity + "/" + entry->dexterity;
        violateRestrictions = true;
    }
    if (entry->strength > currentPlayerData.strength) {
        s += "\nsila: " + (String) currentPlayerData.strength + "/" + (String) entry->strength;
        violateRestrictions = true;
    }
    if (violateRestrictions) {
        String message = "Nedostatecna\n---------------\n";
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

    Debug.printf("Charging item: name=%s strength=%d magic=%d dexterity=%d skill=%d bonus_points=%d\n",
                 entry->name.c_str(), entry->strength, entry->magic, entry->dexterity, entry->skill,
                 entry->bonus_points);

    const Transaction t = Transaction{
            .time = framework->clocks.getCurrentTime(),
            .user_id= static_cast<u8>(currentPlayerData.user_id),
            .skill = entry->skill
    };

    if (!framework->storage.appendTransaction(t)) {
        Debug.println("Can't write to the commit log!");
        return false;
    }

    std::set<portal_Skill> skills;

    for (int i = 0; i < currentPlayerData.skills_count; i++) {
        Serial.printf("Reading skill: %d\n", currentPlayerData.skills[i]);
        skills.insert(currentPlayerData.skills[i]);
    }

    if (entry->skill > 0) {
        Serial.printf("Inserting skill: %d\n", entry->skill);
        skills.insert(static_cast<_portal_Skill>(entry->skill));
    } else if (entry->skill < 0) {
        Serial.printf("Deleting skill: %d\n", entry->skill);
        skills.erase(static_cast<_portal_Skill>(-entry->skill));
    }

    skills.erase(portal_Skill_Skill_Default);

    for (int i = 0; i < currentPlayerData.skills_count; i++) {
        currentPlayerData.skills[i] = portal_Skill_Skill_Default;
    }

    int i = 0;
    for (auto skill: skills) {
        Serial.printf("Writing skill %d to position %d\n", skill, i);
        currentPlayerData.skills[i] = skill;
        i++;
    }

    currentPlayerData.skills_count = i;
//    currentPlayerData.strength += entry->strength;
//    currentPlayerData.strength += entry->magic;
//    currentPlayerData.strength += entry->dexterity;


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
