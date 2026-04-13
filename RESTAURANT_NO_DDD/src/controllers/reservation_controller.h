#pragma once
#include "services/reservation_service.h"
#include <httplib.h>

class ReservationController {
public:
    explicit ReservationController(ReservationService& svc);
    void registerRoutes(httplib::Server& server);

private:
    ReservationService& svc_;
};
