#pragma once
#include "shared_kernel/domain_event.h"
#include <string>

namespace floor_ctx {

struct WalkInSeated : shared::DomainEvent {
    shared::Id tableId;
    int coverCount;
    WalkInSeated(shared::Id t, int c)
        : DomainEvent("WalkInSeated")
        , tableId(std::move(t))
        , coverCount(c) {}
};

struct ReservationSeated : shared::DomainEvent {
    shared::Id tableId;
    std::string name;
    int coverCount;
    ReservationSeated(shared::Id t, std::string n, int c)
        : DomainEvent("ReservationSeated")
        , tableId(std::move(t))
        , name(std::move(n))
        , coverCount(c) {}
};

struct TableTurned : shared::DomainEvent {
    shared::Id tableId;
    int coverCount;
    TableTurned(shared::Id t, int c)
        : DomainEvent("TableTurned")
        , tableId(std::move(t))
        , coverCount(c) {}
};

struct MenuItemSoldOut : shared::DomainEvent {
    shared::Id menuItemId;
    std::string name;
    MenuItemSoldOut(shared::Id m, std::string n)
        : DomainEvent("MenuItemSoldOut")
        , menuItemId(std::move(m))
        , name(std::move(n)) {}
};

} // namespace floor_ctx
