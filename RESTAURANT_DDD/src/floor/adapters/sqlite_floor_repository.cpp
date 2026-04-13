#include "floor/adapters/sqlite_floor_repository.h"
#include <sqlite3.h>
#include <stdexcept>

namespace floor_ctx {

SqliteFloorRepository::SqliteFloorRepository(Database& db) : db_(db) {}

// ---------------------------------------------------------------------------
// Table persistence — maps to shared `tables` table
// ---------------------------------------------------------------------------

void SqliteFloorRepository::saveTable(const Table& table) {
    const char* sql =
        "INSERT INTO tables (id, table_number, capacity, status) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveTable prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    auto statusStr = tableStatusToString(table.status());
    sqlite3_bind_text(stmt, 1, table.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, table.tableNumber());
    sqlite3_bind_int(stmt, 3, table.capacity());
    sqlite3_bind_text(stmt, 4, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveTable step: " + err);
    }
    sqlite3_finalize(stmt);
}

void SqliteFloorRepository::updateTable(const Table& table) {
    const char* sql =
        "UPDATE tables SET table_number = ?, capacity = ?, status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateTable prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    auto statusStr = tableStatusToString(table.status());
    sqlite3_bind_int(stmt, 1, table.tableNumber());
    sqlite3_bind_int(stmt, 2, table.capacity());
    sqlite3_bind_text(stmt, 3, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, table.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateTable step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<Table> SqliteFloorRepository::findTableById(const shared::Id& id) {
    const char* sql =
        "SELECT id, table_number, capacity, status FROM tables WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findTableById prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto table = Table(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            sqlite3_column_int(stmt, 1),
            sqlite3_column_int(stmt, 2),
            tableStatusFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))
        );
        sqlite3_finalize(stmt);
        return table;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Table> SqliteFloorRepository::findAllTables() {
    const char* sql = "SELECT id, table_number, capacity, status FROM tables;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllTables prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<Table> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            sqlite3_column_int(stmt, 1),
            sqlite3_column_int(stmt, 2),
            tableStatusFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))
        );
    }
    sqlite3_finalize(stmt);
    return tables;
}

// ---------------------------------------------------------------------------
// MenuItem persistence — anti-corruption layer over shared `items` table
// Sara sees: id, name, description, price_cents, is_available
// She ignores: prep_time, cooking_method, station, costs
// ---------------------------------------------------------------------------

void SqliteFloorRepository::saveMenuItem(const MenuItem& item) {
    const char* sql =
        "INSERT INTO items (id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) "
        "VALUES (?, ?, ?, ?, 0, '', '', 0, 0, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveMenuItem prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, item.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, item.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, item.description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, item.price().cents);
    sqlite3_bind_int(stmt, 5, item.isAvailable() ? 1 : 0);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveMenuItem step: " + err);
    }
    sqlite3_finalize(stmt);
}

void SqliteFloorRepository::updateMenuItem(const MenuItem& item) {
    // Only update floor fields — never touch kitchen fields
    const char* sql =
        "UPDATE items SET name = ?, description = ?, price_cents = ?, is_available = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateMenuItem prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, item.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, item.description().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, item.price().cents);
    sqlite3_bind_int(stmt, 4, item.isAvailable() ? 1 : 0);
    sqlite3_bind_text(stmt, 5, item.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateMenuItem step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<MenuItem> SqliteFloorRepository::findMenuItemById(const shared::Id& id) {
    const char* sql =
        "SELECT id, name, description, price_cents, is_available FROM items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findMenuItemById prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto descRaw = sqlite3_column_text(stmt, 2);
        std::string desc = descRaw ? reinterpret_cast<const char*>(descRaw) : "";
        auto item = MenuItem(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            desc,
            shared::Money(sqlite3_column_int(stmt, 3)),
            sqlite3_column_int(stmt, 4) != 0
        );
        sqlite3_finalize(stmt);
        return item;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<MenuItem> SqliteFloorRepository::findMenuItemByName(const std::string& name) {
    const char* sql =
        "SELECT id, name, description, price_cents, is_available FROM items WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findMenuItemByName prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto descRaw = sqlite3_column_text(stmt, 2);
        std::string desc = descRaw ? reinterpret_cast<const char*>(descRaw) : "";
        auto item = MenuItem(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            desc,
            shared::Money(sqlite3_column_int(stmt, 3)),
            sqlite3_column_int(stmt, 4) != 0
        );
        sqlite3_finalize(stmt);
        return item;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<MenuItem> SqliteFloorRepository::findAllMenuItems() {
    const char* sql =
        "SELECT id, name, description, price_cents, is_available FROM items;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllMenuItems prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<MenuItem> items;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto descRaw = sqlite3_column_text(stmt, 2);
        std::string desc = descRaw ? reinterpret_cast<const char*>(descRaw) : "";
        items.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            desc,
            shared::Money(sqlite3_column_int(stmt, 3)),
            sqlite3_column_int(stmt, 4) != 0
        );
    }
    sqlite3_finalize(stmt);
    return items;
}

// ---------------------------------------------------------------------------
// Seating persistence — maps to shared `seatings` table
// ---------------------------------------------------------------------------

void SqliteFloorRepository::saveSeating(const Seating& seating) {
    const char* sql =
        "INSERT INTO seatings (id, table_id, cover_count, is_walk_in, reservation_name, "
        "seated_at, cleared_at) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveSeating prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, seating.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, seating.tableId().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, seating.coverCount());
    sqlite3_bind_int(stmt, 4, seating.isWalkIn() ? 1 : 0);
    sqlite3_bind_text(stmt, 5, seating.reservationName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, seating.seatedAt().value.c_str(), -1, SQLITE_TRANSIENT);
    if (seating.clearedAt().empty()) {
        sqlite3_bind_null(stmt, 7);
    } else {
        sqlite3_bind_text(stmt, 7, seating.clearedAt().value.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveSeating step: " + err);
    }
    sqlite3_finalize(stmt);
}

void SqliteFloorRepository::updateSeating(const Seating& seating) {
    const char* sql =
        "UPDATE seatings SET cover_count = ?, is_walk_in = ?, reservation_name = ?, "
        "seated_at = ?, cleared_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateSeating prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_int(stmt, 1, seating.coverCount());
    sqlite3_bind_int(stmt, 2, seating.isWalkIn() ? 1 : 0);
    sqlite3_bind_text(stmt, 3, seating.reservationName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, seating.seatedAt().value.c_str(), -1, SQLITE_TRANSIENT);
    if (seating.clearedAt().empty()) {
        sqlite3_bind_null(stmt, 5);
    } else {
        sqlite3_bind_text(stmt, 5, seating.clearedAt().value.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_text(stmt, 6, seating.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateSeating step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<Seating> SqliteFloorRepository::findActiveSeatingByTableId(const shared::Id& tableId) {
    const char* sql =
        "SELECT id, table_id, cover_count, is_walk_in, reservation_name, seated_at, cleared_at "
        "FROM seatings WHERE table_id = ? AND cleared_at IS NULL;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findActiveSeatingByTableId prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, tableId.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto nameRaw = sqlite3_column_text(stmt, 4);
        std::string name = nameRaw ? reinterpret_cast<const char*>(nameRaw) : "";
        auto clearedRaw = sqlite3_column_text(stmt, 6);
        std::string cleared = clearedRaw ? reinterpret_cast<const char*>(clearedRaw) : "";
        auto seating = Seating(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
            sqlite3_column_int(stmt, 2),
            sqlite3_column_int(stmt, 3) != 0,
            name,
            shared::Timestamp{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))},
            shared::Timestamp{cleared}
        );
        sqlite3_finalize(stmt);
        return seating;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

int SqliteFloorRepository::countCoversToday() {
    const char* sql =
        "SELECT COALESCE(SUM(cover_count), 0) FROM seatings WHERE date(seated_at) = date('now');";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("countCoversToday prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

} // namespace floor_ctx
