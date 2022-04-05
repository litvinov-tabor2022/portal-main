#ifndef PORTAL_PRICELIST_H
#define PORTAL_PRICELIST_H

#include <map>
#include "defs.h"

class PriceList {
public:
    bool load(PortalRole portalRole, Storage *storage);
    PriceListEntry* getByCode(const String& code);

private:
    static String portalRoleToName(PortalRole portalRole) {
        switch (portalRole) {
            case Default:
                return "default";
            case Skills:
                return "skills";
        }
    }

    std::map<String, PriceListEntry> prices;
};


#endif //PORTAL_PRICELIST_H
