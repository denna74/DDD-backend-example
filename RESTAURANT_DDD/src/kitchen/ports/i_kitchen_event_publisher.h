#pragma once
#include "kitchen/domain/kitchen_events.h"

namespace kitchen {

class IKitchenEventPublisher {
public:
    virtual ~IKitchenEventPublisher() = default;
    virtual void publishDishFired(const DishFired&) = 0;
    virtual void publishDishPlated(const DishPlated&) = 0;
    virtual void publishAllDishesPlated(const AllDishesPlated&) = 0;
    virtual void publishDishMarkedOutOfStock(const DishMarkedOutOfStock&) = 0;
};

} // namespace kitchen
