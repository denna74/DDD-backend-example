#pragma once
#include "floor/application/floor_service.h"
#include <httplib.h>

namespace floor_ctx {

class HttpFloorController {
public:
    HttpFloorController(FloorService& service);
    void registerRoutes(httplib::Server& server);

private:
    FloorService& service_;
};

} // namespace floor_ctx
