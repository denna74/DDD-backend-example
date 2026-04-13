#pragma once
#include "kitchen/ports/i_kitchen_repository.h"
#include "shared_kernel/db.h"

namespace kitchen {

class SqliteKitchenRepository : public IKitchenRepository {
public:
    explicit SqliteKitchenRepository(Database& db);

    void saveDish(const Dish& dish) override;
    void updateDish(const Dish& dish) override;
    std::optional<Dish> findDishById(const shared::Id& id) override;
    std::optional<Dish> findDishByName(const std::string& name) override;
    std::vector<Dish> findAllDishes() override;

    void saveFireOrder(const FireOrder& order) override;
    void updateFireOrder(const FireOrder& order) override;
    std::optional<FireOrder> findFireOrderById(const shared::Id& id) override;
    std::vector<FireOrder> findAllFireOrders() override;

private:
    Database& db_;
};

} // namespace kitchen
