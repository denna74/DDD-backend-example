#pragma once
#include "kitchen/ports/i_kitchen_repository.h"
#include "kitchen/ports/i_kitchen_event_publisher.h"
#include "shared_kernel/types.h"
#include <string>
#include <vector>
#include <optional>

namespace kitchen {

struct CreateFireOrderRequest {
    int tableNumber;
    struct DishTiming {
        std::string dishId;
        int fireAtOffsetMinutes;
    };
    std::vector<DishTiming> dishes;
};

class KitchenService {
public:
    KitchenService(IKitchenRepository& repo, IKitchenEventPublisher& events);

    std::vector<Dish> listDishes();
    std::optional<Dish> getDish(const std::string& id);
    std::string addDish(const std::string& name, int prepTimeMinutes,
                        const std::string& cookingMethod, const std::string& station);
    shared::Result<void> markDishOutOfStock(const std::string& id);
    shared::Result<void> restoreDish(const std::string& id);

    std::string createFireOrder(CreateFireOrderRequest& request);
    std::vector<FireOrder> listFireOrders();
    std::optional<FireOrder> getFireOrder(const std::string& id);
    shared::Result<void> fireDish(const std::string& fireOrderId, const std::string& lineId);
    shared::Result<void> plateDish(const std::string& fireOrderId, const std::string& lineId);

private:
    IKitchenRepository& repo_;
    IKitchenEventPublisher& events_;
};

} // namespace kitchen
