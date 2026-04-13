#pragma once
#include "shared_kernel/domain_event.h"
#include <string>

namespace kitchen {

struct DishFired : shared::DomainEvent {
    shared::Id fireOrderId, lineId, dishId;
    DishFired(shared::Id fo, shared::Id l, shared::Id d)
        : DomainEvent("DishFired")
        , fireOrderId(std::move(fo))
        , lineId(std::move(l))
        , dishId(std::move(d)) {}
};

struct DishPlated : shared::DomainEvent {
    shared::Id fireOrderId, lineId;
    DishPlated(shared::Id fo, shared::Id l)
        : DomainEvent("DishPlated")
        , fireOrderId(std::move(fo))
        , lineId(std::move(l)) {}
};

struct AllDishesPlated : shared::DomainEvent {
    int tableNumber;
    AllDishesPlated(int t)
        : DomainEvent("AllDishesPlated")
        , tableNumber(t) {}
};

struct DishMarkedOutOfStock : shared::DomainEvent {
    shared::Id dishId;
    std::string dishName;
    DishMarkedOutOfStock(shared::Id d, std::string n)
        : DomainEvent("DishMarkedOutOfStock")
        , dishId(std::move(d))
        , dishName(std::move(n)) {}
};

} // namespace kitchen
