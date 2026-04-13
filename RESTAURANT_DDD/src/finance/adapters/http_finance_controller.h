#pragma once
#include "finance/application/finance_service.h"
#include <httplib.h>

namespace finance {

class HttpFinanceController {
public:
    HttpFinanceController(FinanceService& service);
    void registerRoutes(httplib::Server& server);

private:
    FinanceService& service_;
};

} // namespace finance
