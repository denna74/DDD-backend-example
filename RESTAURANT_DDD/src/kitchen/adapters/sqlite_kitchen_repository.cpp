#include "kitchen/adapters/sqlite_kitchen_repository.h"
#include <sqlite3.h>
#include <stdexcept>

namespace kitchen {

SqliteKitchenRepository::SqliteKitchenRepository(Database& db) : db_(db) {}

// ---------------------------------------------------------------------------
// Dish persistence — anti-corruption layer over shared `items` table
// ---------------------------------------------------------------------------

void SqliteKitchenRepository::saveDish(const Dish& dish) {
    const char* sql =
        "INSERT INTO items (id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) "
        "VALUES (?, ?, '', 0, ?, ?, ?, 0, 0, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveDish prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, dish.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dish.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, dish.prepTimeMinutes());
    sqlite3_bind_text(stmt, 4, dish.cookingMethod().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, stationToString(dish.station()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, dish.isAvailable() ? 1 : 0);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveDish step: " + err);
    }
    sqlite3_finalize(stmt);
}

void SqliteKitchenRepository::updateDish(const Dish& dish) {
    const char* sql =
        "UPDATE items SET name = ?, prep_time_minutes = ?, cooking_method = ?, "
        "station = ?, is_available = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateDish prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, dish.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, dish.prepTimeMinutes());
    sqlite3_bind_text(stmt, 3, dish.cookingMethod().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, stationToString(dish.station()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, dish.isAvailable() ? 1 : 0);
    sqlite3_bind_text(stmt, 6, dish.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateDish step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<Dish> SqliteKitchenRepository::findDishById(const shared::Id& id) {
    const char* sql =
        "SELECT id, name, prep_time_minutes, cooking_method, station, is_available "
        "FROM items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findDishById prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto dish = Dish(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            stationFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))),
            sqlite3_column_int(stmt, 5) != 0
        );
        sqlite3_finalize(stmt);
        return dish;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<Dish> SqliteKitchenRepository::findDishByName(const std::string& name) {
    const char* sql =
        "SELECT id, name, prep_time_minutes, cooking_method, station, is_available "
        "FROM items WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findDishByName prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto dish = Dish(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            stationFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))),
            sqlite3_column_int(stmt, 5) != 0
        );
        sqlite3_finalize(stmt);
        return dish;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Dish> SqliteKitchenRepository::findAllDishes() {
    const char* sql =
        "SELECT id, name, prep_time_minutes, cooking_method, station, is_available FROM items;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllDishes prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<Dish> dishes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        dishes.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            sqlite3_column_int(stmt, 2),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            stationFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))),
            sqlite3_column_int(stmt, 5) != 0
        );
    }
    sqlite3_finalize(stmt);
    return dishes;
}

// ---------------------------------------------------------------------------
// FireOrder persistence — maps to shared `orders` + `order_lines` tables
// ---------------------------------------------------------------------------

void SqliteKitchenRepository::saveFireOrder(const FireOrder& order) {
    // Insert into orders table
    const char* orderSql =
        "INSERT INTO orders (id, table_number, status, created_at) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), orderSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveFireOrder prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    auto statusStr = fireOrderStatusToString(order.status());
    sqlite3_bind_text(stmt, 1, order.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, order.tableNumber());
    sqlite3_bind_text(stmt, 3, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, order.createdAt().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveFireOrder step: " + err);
    }
    sqlite3_finalize(stmt);

    // Insert lines into order_lines table
    const char* lineSql =
        "INSERT INTO order_lines (id, order_id, item_id, status, fire_at_offset_minutes, fired_at, plated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";
    for (const auto& line : order.lines()) {
        if (sqlite3_prepare_v2(db_.handle(), lineSql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(std::string("saveFireOrder line prepare: ") + sqlite3_errmsg(db_.handle()));
        }
        auto lineStatusStr = fireLineStatusToString(line.status());
        sqlite3_bind_text(stmt, 1, line.id().value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, order.id().value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, line.dishId().value.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, lineStatusStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, line.fireAtOffsetMinutes());
        if (line.firedAt().empty()) {
            sqlite3_bind_null(stmt, 6);
        } else {
            sqlite3_bind_text(stmt, 6, line.firedAt().value.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (line.platedAt().empty()) {
            sqlite3_bind_null(stmt, 7);
        } else {
            sqlite3_bind_text(stmt, 7, line.platedAt().value.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("saveFireOrder line step: " + err);
        }
        sqlite3_finalize(stmt);
    }
}

void SqliteKitchenRepository::updateFireOrder(const FireOrder& order) {
    // Update order status
    const char* orderSql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), orderSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateFireOrder prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    auto statusStr = fireOrderStatusToString(order.status());
    sqlite3_bind_text(stmt, 1, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, order.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateFireOrder step: " + err);
    }
    sqlite3_finalize(stmt);

    // Update each line
    const char* lineSql =
        "UPDATE order_lines SET status = ?, fired_at = ?, plated_at = ? WHERE id = ?;";
    for (const auto& line : order.lines()) {
        if (sqlite3_prepare_v2(db_.handle(), lineSql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(std::string("updateFireOrder line prepare: ") + sqlite3_errmsg(db_.handle()));
        }
        auto lineStatusStr = fireLineStatusToString(line.status());
        sqlite3_bind_text(stmt, 1, lineStatusStr.c_str(), -1, SQLITE_TRANSIENT);
        if (line.firedAt().empty()) {
            sqlite3_bind_null(stmt, 2);
        } else {
            sqlite3_bind_text(stmt, 2, line.firedAt().value.c_str(), -1, SQLITE_TRANSIENT);
        }
        if (line.platedAt().empty()) {
            sqlite3_bind_null(stmt, 3);
        } else {
            sqlite3_bind_text(stmt, 3, line.platedAt().value.c_str(), -1, SQLITE_TRANSIENT);
        }
        sqlite3_bind_text(stmt, 4, line.id().value.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("updateFireOrder line step: " + err);
        }
        sqlite3_finalize(stmt);
    }
}

std::optional<FireOrder> SqliteKitchenRepository::findFireOrderById(const shared::Id& id) {
    const char* orderSql =
        "SELECT id, table_number, status, created_at FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), orderSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findFireOrderById prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    auto orderId = shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
    int tableNumber = sqlite3_column_int(stmt, 1);
    std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    auto createdAt = shared::Timestamp{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))};
    sqlite3_finalize(stmt);

    FireOrder order(orderId, tableNumber, createdAt);

    // Load lines
    const char* lineSql =
        "SELECT id, item_id, status, fire_at_offset_minutes, fired_at, plated_at "
        "FROM order_lines WHERE order_id = ?;";
    if (sqlite3_prepare_v2(db_.handle(), lineSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findFireOrderById lines prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto lineId = shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
        auto dishId = shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))};
        std::string lineStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int offset = sqlite3_column_int(stmt, 3);

        order.addLine(lineId, dishId, offset);

        // Restore state: fire and plate lines based on stored status
        auto& addedLine = order.lines().back();
        // We need to use fireDish/plateDish to transition states, but they look up by lineId.
        // Instead, we'll reconstruct via the aggregate methods after adding all lines.
        // Store info for later restoration.
        (void)addedLine;
        (void)lineStatus;
    }
    sqlite3_finalize(stmt);

    // Re-read lines to restore their states via aggregate commands
    // We need a second pass because we need all lines added first.
    // Actually, let's re-read and use fireDish/plateDish on the aggregate.
    if (sqlite3_prepare_v2(db_.handle(), lineSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findFireOrderById restore prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto lineId = shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
        std::string lineStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if (lineStatus == "fired") {
            order.fireDish(lineId);
        } else if (lineStatus == "plated") {
            order.fireDish(lineId); // must fire before plating
            order.plateDish(lineId);
        }
    }
    sqlite3_finalize(stmt);

    return order;
}

std::vector<FireOrder> SqliteKitchenRepository::findAllFireOrders() {
    const char* sql = "SELECT id FROM orders;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllFireOrders prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<shared::Id> ids;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ids.push_back(shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))});
    }
    sqlite3_finalize(stmt);

    std::vector<FireOrder> orders;
    for (const auto& id : ids) {
        auto order = findFireOrderById(id);
        if (order) {
            orders.push_back(std::move(*order));
        }
    }
    return orders;
}

} // namespace kitchen
