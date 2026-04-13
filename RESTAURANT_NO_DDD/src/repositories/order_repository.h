#pragma once
#include "models/order.h"
#include "models/order_line.h"
#include "db.h"
#include <vector>
#include <optional>

struct OrderWithLines {
    Order order;
    std::vector<OrderLine> lines;
};

class OrderRepository {
public:
    explicit OrderRepository(Database& db) : db_(db) {}
    void create(const Order& order, const std::vector<OrderLine>& lines);
    std::optional<OrderWithLines> findById(const std::string& id);
    std::vector<OrderWithLines> findAll();
    void updateStatus(const std::string& id, const std::string& status);
private:
    Database& db_;
    static Order rowToOrder(sqlite3_stmt* stmt);
    static OrderLine rowToOrderLine(sqlite3_stmt* stmt);
};
