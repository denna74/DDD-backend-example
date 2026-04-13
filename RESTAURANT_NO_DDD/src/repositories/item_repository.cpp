#include "repositories/item_repository.h"
#include <stdexcept>

Item ItemRepository::rowToItem(sqlite3_stmt* stmt) {
    Item item;
    item.id              = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    item.name            = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    auto desc            = sqlite3_column_text(stmt, 2);
    item.description     = desc ? reinterpret_cast<const char*>(desc) : "";
    item.price_cents           = sqlite3_column_int(stmt, 3);
    item.prep_time_minutes     = sqlite3_column_int(stmt, 4);
    item.cooking_method  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    item.station         = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    item.ingredient_cost_cents = sqlite3_column_int(stmt, 7);
    item.supplier_price_cents  = sqlite3_column_int(stmt, 8);
    item.is_available    = sqlite3_column_int(stmt, 9) != 0;
    return item;
}

void ItemRepository::create(const Item& item) {
    const char* sql =
        "INSERT INTO items (id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ItemRepository::create prepare failed");
    }
    sqlite3_bind_text(stmt, 1, item.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, item.name.c_str(), -1, SQLITE_STATIC);
    if (item.description.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, item.description.c_str(), -1, SQLITE_STATIC);
    }
    sqlite3_bind_int(stmt, 4, item.price_cents);
    sqlite3_bind_int(stmt, 5, item.prep_time_minutes);
    sqlite3_bind_text(stmt, 6, item.cooking_method.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, item.station.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, item.ingredient_cost_cents);
    sqlite3_bind_int(stmt, 9, item.supplier_price_cents);
    sqlite3_bind_int(stmt, 10, item.is_available ? 1 : 0);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<Item> ItemRepository::findById(const std::string& id) {
    const char* sql =
        "SELECT id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available "
        "FROM items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ItemRepository::findById prepare failed");
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    std::optional<Item> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToItem(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Item> ItemRepository::findAll() {
    const char* sql =
        "SELECT id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available "
        "FROM items;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ItemRepository::findAll prepare failed");
    }
    std::vector<Item> items;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        items.push_back(rowToItem(stmt));
    }
    sqlite3_finalize(stmt);
    return items;
}

void ItemRepository::update(const Item& item) {
    const char* sql =
        "UPDATE items SET name=?, description=?, price_cents=?, prep_time_minutes=?, "
        "cooking_method=?, station=?, ingredient_cost_cents=?, supplier_price_cents=?, "
        "is_available=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ItemRepository::update prepare failed");
    }
    sqlite3_bind_text(stmt, 1, item.name.c_str(), -1, SQLITE_STATIC);
    if (item.description.empty()) {
        sqlite3_bind_null(stmt, 2);
    } else {
        sqlite3_bind_text(stmt, 2, item.description.c_str(), -1, SQLITE_STATIC);
    }
    sqlite3_bind_int(stmt, 3, item.price_cents);
    sqlite3_bind_int(stmt, 4, item.prep_time_minutes);
    sqlite3_bind_text(stmt, 5, item.cooking_method.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, item.station.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, item.ingredient_cost_cents);
    sqlite3_bind_int(stmt, 8, item.supplier_price_cents);
    sqlite3_bind_int(stmt, 9, item.is_available ? 1 : 0);
    sqlite3_bind_text(stmt, 10, item.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void ItemRepository::updateAvailability(const std::string& id, bool available) {
    const char* sql = "UPDATE items SET is_available=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ItemRepository::updateAvailability prepare failed");
    }
    sqlite3_bind_int(stmt, 1, available ? 1 : 0);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
