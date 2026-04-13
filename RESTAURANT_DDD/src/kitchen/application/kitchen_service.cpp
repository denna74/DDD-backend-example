#include "kitchen/application/kitchen_service.h"

namespace kitchen {

KitchenService::KitchenService(IKitchenRepository& repo, IKitchenEventPublisher& events)
    : repo_(repo), events_(events) {}

std::vector<Dish> KitchenService::listDishes() {
    return repo_.findAllDishes();
}

std::optional<Dish> KitchenService::getDish(const std::string& id) {
    return repo_.findDishById(shared::Id{id});
}

std::string KitchenService::addDish(const std::string& name, int prepTimeMinutes,
                                     const std::string& cookingMethod, const std::string& station) {
    auto id = shared::Id::generate();
    Dish dish(id, name, prepTimeMinutes, cookingMethod, stationFromString(station));
    repo_.saveDish(dish);
    return id.value;
}

shared::Result<void> KitchenService::markDishOutOfStock(const std::string& id) {
    auto dish = repo_.findDishById(shared::Id{id});
    if (!dish) {
        return shared::Result<void>::fail("Dish not found");
    }
    dish->markOutOfStock();
    repo_.updateDish(*dish);
    events_.publishDishMarkedOutOfStock(
        DishMarkedOutOfStock(dish->id(), dish->name()));
    return shared::Result<void>::ok();
}

shared::Result<void> KitchenService::restoreDish(const std::string& id) {
    auto dish = repo_.findDishById(shared::Id{id});
    if (!dish) {
        return shared::Result<void>::fail("Dish not found");
    }
    dish->restore();
    repo_.updateDish(*dish);
    return shared::Result<void>::ok();
}

std::string KitchenService::createFireOrder(CreateFireOrderRequest& request) {
    auto orderId = shared::Id::generate();
    FireOrder order(orderId, request.tableNumber, shared::Timestamp::now());
    for (const auto& dt : request.dishes) {
        auto lineId = shared::Id::generate();
        order.addLine(lineId, shared::Id{dt.dishId}, dt.fireAtOffsetMinutes);
    }
    repo_.saveFireOrder(order);
    return orderId.value;
}

std::vector<FireOrder> KitchenService::listFireOrders() {
    return repo_.findAllFireOrders();
}

std::optional<FireOrder> KitchenService::getFireOrder(const std::string& id) {
    return repo_.findFireOrderById(shared::Id{id});
}

shared::Result<void> KitchenService::fireDish(const std::string& fireOrderId, const std::string& lineId) {
    auto order = repo_.findFireOrderById(shared::Id{fireOrderId});
    if (!order) {
        return shared::Result<void>::fail("Fire order not found");
    }
    auto result = order->fireDish(shared::Id{lineId});
    if (result.isError()) {
        return result;
    }
    repo_.updateFireOrder(*order);

    // Find the dish ID for the fired line
    for (const auto& line : order->lines()) {
        if (line.id() == shared::Id{lineId}) {
            events_.publishDishFired(DishFired(order->id(), line.id(), line.dishId()));
            break;
        }
    }
    return shared::Result<void>::ok();
}

shared::Result<void> KitchenService::plateDish(const std::string& fireOrderId, const std::string& lineId) {
    auto order = repo_.findFireOrderById(shared::Id{fireOrderId});
    if (!order) {
        return shared::Result<void>::fail("Fire order not found");
    }
    auto result = order->plateDish(shared::Id{lineId});
    if (result.isError()) {
        return result;
    }
    repo_.updateFireOrder(*order);

    events_.publishDishPlated(DishPlated(order->id(), shared::Id{lineId}));

    if (order->status() == FireOrderStatus::Plated) {
        events_.publishAllDishesPlated(AllDishesPlated(order->tableNumber()));
    }
    return shared::Result<void>::ok();
}

} // namespace kitchen
