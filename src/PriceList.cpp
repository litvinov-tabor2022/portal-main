#include "PriceList.h"
#include "Constants.h"
#include "ArduinoJson.h"

bool PriceList::load(PortalRole portalRole, Storage *storage) {
    DynamicJsonDocument *json = storage->loadJsonFile(PRICELIST_FILE);
    if (json == nullptr) return false;

    JsonArray list = (*json)[portalRoleToName(portalRole)];

    this->prices.clear();

    for (auto row: list) {
        const PriceListEntry entry = PriceListEntry{
                .code =  row["code"],
                .name =  row["name"],
                .strength =  row["strength"],
                .magic = row["magic"],
                .dexterity = row["dexterity"],
                .skill =  row["skill"],
                .bonus_points = row["bonus_points"]
        };

        Debug.printf("Loaded entry: name=%s code=%s strength=%d magic=%d dexterity=%d experiences=%d\n",
                     entry.name.c_str(), entry.code.c_str(), entry.strength, entry.magic, entry.dexterity, entry.bonus_points);

        this->prices.insert(std::make_pair(entry.code, entry));
    }

    return true;
}

PriceListEntry *PriceList::getByCode(const String &code) {
    auto it = this->prices.find(code);
    if (it == prices.end()) return nullptr;

    return &it->second;
}
