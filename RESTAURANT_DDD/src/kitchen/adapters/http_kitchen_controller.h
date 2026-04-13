#pragma once
#include "kitchen/application/kitchen_service.h"
#include <httplib.h>

namespace kitchen {

class HttpKitchenController {
public:
    HttpKitchenController(KitchenService& service);
    void registerRoutes(httplib::Server& server);

private:
    KitchenService& service_;
};

} // namespace kitchen
