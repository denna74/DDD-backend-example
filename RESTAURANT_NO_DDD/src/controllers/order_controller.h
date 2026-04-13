#pragma once
#include "services/order_service.h"
#include <httplib.h>

class OrderController {
public:
    explicit OrderController(OrderService& svc);
    void registerRoutes(httplib::Server& server);

private:
    OrderService& svc_;
};
