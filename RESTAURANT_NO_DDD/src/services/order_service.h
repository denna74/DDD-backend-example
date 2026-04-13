#pragma once
#include "repositories/order_repository.h"
#include <string>
#include <vector>
#include <optional>

class OrderService {
public:
    explicit OrderService(OrderRepository& repo);

    std::string createOrder(int table_number, const std::vector<std::string>& item_ids);
    std::vector<OrderWithLines> getOrders();
    std::optional<OrderWithLines> getOrder(const std::string& id);
    void updateOrderStatus(const std::string& id, const std::string& status);

private:
    OrderRepository& repo_;
    static std::string generateId();
    static std::string currentTimestamp();
};
