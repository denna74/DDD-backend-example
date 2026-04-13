#pragma once
#include "kitchen/domain/dish.h"
#include "kitchen/domain/fire_order.h"
#include <vector>
#include <optional>

namespace kitchen {

class IKitchenRepository {
public:
    virtual ~IKitchenRepository() = default;
    virtual void saveDish(const Dish& dish) = 0;
    virtual void updateDish(const Dish& dish) = 0;
    virtual std::optional<Dish> findDishById(const shared::Id& id) = 0;
    virtual std::optional<Dish> findDishByName(const std::string& name) = 0;
    virtual std::vector<Dish> findAllDishes() = 0;
    virtual void saveFireOrder(const FireOrder& order) = 0;
    virtual void updateFireOrder(const FireOrder& order) = 0;
    virtual std::optional<FireOrder> findFireOrderById(const shared::Id& id) = 0;
    virtual std::vector<FireOrder> findAllFireOrders() = 0;
};

} // namespace kitchen
