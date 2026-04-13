#pragma once
#include "services/item_service.h"
#include <httplib.h>

class ItemController {
public:
    explicit ItemController(ItemService& svc);
    void registerRoutes(httplib::Server& server);

private:
    ItemService& svc_;
};
