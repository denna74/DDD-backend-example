#pragma once
#include "floor/domain/floor_events.h"

namespace floor_ctx {

class IFloorEventPublisher {
public:
    virtual ~IFloorEventPublisher() = default;
    virtual void publishWalkInSeated(const WalkInSeated&) = 0;
    virtual void publishReservationSeated(const ReservationSeated&) = 0;
    virtual void publishTableTurned(const TableTurned&) = 0;
    virtual void publishMenuItemSoldOut(const MenuItemSoldOut&) = 0;
};

} // namespace floor_ctx
