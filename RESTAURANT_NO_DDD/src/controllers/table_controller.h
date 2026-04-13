#pragma once
#include "services/table_service.h"
#include <httplib.h>

class TableController {
public:
    explicit TableController(TableService& svc);
    void registerRoutes(httplib::Server& server);

private:
    TableService& svc_;
};
