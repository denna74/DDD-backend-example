#include "repositories/order_repository.h"
#include <stdexcept>

Order OrderRepository::rowToOrder(sqlite3_stmt* stmt) {
    Order order;
    order.id           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    order.table_number = sqlite3_column_int(stmt, 1);
    order.status       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    auto ca            = sqlite3_column_text(stmt, 3);
    order.created_at   = ca ? reinterpret_cast<const char*>(ca) : "";
    return order;
}

OrderLine OrderRepository::rowToOrderLine(sqlite3_stmt* stmt) {
    OrderLine line;
    line.id       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    line.order_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    line.item_id  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    line.status   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
        line.fire_at_offset_minutes = sqlite3_column_int(stmt, 4);
    }
    auto fired   = sqlite3_column_text(stmt, 5);
    line.fired_at  = fired  ? reinterpret_cast<const char*>(fired)  : "";
    auto plated  = sqlite3_column_text(stmt, 6);
    line.plated_at = plated ? reinterpret_cast<const char*>(plated) : "";
    return line;
}

void OrderRepository::create(const Order& order, const std::vector<OrderLine>& lines) {
    {
        const char* sql =
            "INSERT INTO orders (id, table_number, status, created_at) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("OrderRepository::create prepare order failed");
        }
        sqlite3_bind_text(stmt, 1, order.id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, order.table_number);
        sqlite3_bind_text(stmt, 3, order.status.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, order.created_at.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    for (const auto& line : lines) {
        const char* sql =
            "INSERT INTO order_lines (id, order_id, item_id, status, fire_at_offset_minutes, "
            "fired_at, plated_at) VALUES (?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("OrderRepository::create prepare line failed");
        }
        sqlite3_bind_text(stmt, 1, line.id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, line.order_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, line.item_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, line.status.c_str(), -1, SQLITE_STATIC);
        if (line.fire_at_offset_minutes.has_value()) {
            sqlite3_bind_int(stmt, 5, line.fire_at_offset_minutes.value());
        } else {
            sqlite3_bind_null(stmt, 5);
        }
        if (line.fired_at.empty()) {
            sqlite3_bind_null(stmt, 6);
        } else {
            sqlite3_bind_text(stmt, 6, line.fired_at.c_str(), -1, SQLITE_STATIC);
        }
        if (line.plated_at.empty()) {
            sqlite3_bind_null(stmt, 7);
        } else {
            sqlite3_bind_text(stmt, 7, line.plated_at.c_str(), -1, SQLITE_STATIC);
        }
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::optional<OrderWithLines> OrderRepository::findById(const std::string& id) {
    const char* sql = "SELECT id, table_number, status, created_at FROM orders WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("OrderRepository::findById prepare failed");
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    std::optional<OrderWithLines> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = OrderWithLines{rowToOrder(stmt), {}};
    }
    sqlite3_finalize(stmt);

    if (!result.has_value()) return result;

    const char* lsql =
        "SELECT id, order_id, item_id, status, fire_at_offset_minutes, fired_at, plated_at "
        "FROM order_lines WHERE order_id=?;";
    sqlite3_stmt* lstmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), lsql, -1, &lstmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("OrderRepository::findById prepare lines failed");
    }
    sqlite3_bind_text(lstmt, 1, id.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(lstmt) == SQLITE_ROW) {
        result->lines.push_back(rowToOrderLine(lstmt));
    }
    sqlite3_finalize(lstmt);
    return result;
}

std::vector<OrderWithLines> OrderRepository::findAll() {
    const char* sql = "SELECT id, table_number, status, created_at FROM orders;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("OrderRepository::findAll prepare failed");
    }
    std::vector<Order> orders;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        orders.push_back(rowToOrder(stmt));
    }
    sqlite3_finalize(stmt);

    std::vector<OrderWithLines> result;
    for (const auto& order : orders) {
        const char* lsql =
            "SELECT id, order_id, item_id, status, fire_at_offset_minutes, fired_at, plated_at "
            "FROM order_lines WHERE order_id=?;";
        sqlite3_stmt* lstmt = nullptr;
        if (sqlite3_prepare_v2(db_.handle(), lsql, -1, &lstmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("OrderRepository::findAll prepare lines failed");
        }
        sqlite3_bind_text(lstmt, 1, order.id.c_str(), -1, SQLITE_STATIC);
        std::vector<OrderLine> lines;
        while (sqlite3_step(lstmt) == SQLITE_ROW) {
            lines.push_back(rowToOrderLine(lstmt));
        }
        sqlite3_finalize(lstmt);
        result.push_back({order, lines});
    }
    return result;
}

void OrderRepository::updateStatus(const std::string& id, const std::string& status) {
    const char* sql = "UPDATE orders SET status=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("OrderRepository::updateStatus prepare failed");
    }
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
