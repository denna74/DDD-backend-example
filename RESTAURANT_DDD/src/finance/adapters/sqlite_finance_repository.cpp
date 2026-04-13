#include "finance/adapters/sqlite_finance_repository.h"
#include <sqlite3.h>
#include <stdexcept>

namespace finance {

SqliteFinanceRepository::SqliteFinanceRepository(Database& db) : db_(db) {}

// ---------------------------------------------------------------------------
// CostItem persistence — anti-corruption layer over shared `items` table
// Maps only cost-relevant fields: id, name, ingredient_cost_cents,
// supplier_price_cents, price_cents (as selling price).
// Ignores prep_time, cooking_method, station, description.
// ---------------------------------------------------------------------------

void SqliteFinanceRepository::saveCostItem(const CostItem& item) {
    const char* sql =
        "INSERT INTO items (id, name, description, price_cents, prep_time_minutes, "
        "cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) "
        "VALUES (?, ?, '', ?, 0, '', '', ?, ?, 1);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveCostItem prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, item.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, item.name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, item.sellingPrice().cents);
    sqlite3_bind_int(stmt, 4, item.ingredientCost().cents);
    sqlite3_bind_int(stmt, 5, item.supplierPrice().cents);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveCostItem step: " + err);
    }
    sqlite3_finalize(stmt);
}

void SqliteFinanceRepository::updateCostItem(const CostItem& item) {
    const char* sql =
        "UPDATE items SET ingredient_cost_cents = ?, supplier_price_cents = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("updateCostItem prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_int(stmt, 1, item.ingredientCost().cents);
    sqlite3_bind_int(stmt, 2, item.supplierPrice().cents);
    sqlite3_bind_text(stmt, 3, item.id().value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("updateCostItem step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<CostItem> SqliteFinanceRepository::findCostItemById(const shared::Id& id) {
    const char* sql =
        "SELECT id, name, ingredient_cost_cents, supplier_price_cents, price_cents "
        "FROM items WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findCostItemById prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.value.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto item = CostItem(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            shared::Money(sqlite3_column_int(stmt, 2)),
            shared::Money(sqlite3_column_int(stmt, 3)),
            shared::Money(sqlite3_column_int(stmt, 4))
        );
        sqlite3_finalize(stmt);
        return item;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<CostItem> SqliteFinanceRepository::findAllCostItems() {
    const char* sql =
        "SELECT id, name, ingredient_cost_cents, supplier_price_cents, price_cents FROM items;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllCostItems prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<CostItem> items;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        items.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            shared::Money(sqlite3_column_int(stmt, 2)),
            shared::Money(sqlite3_column_int(stmt, 3)),
            shared::Money(sqlite3_column_int(stmt, 4))
        );
    }
    sqlite3_finalize(stmt);
    return items;
}

// ---------------------------------------------------------------------------
// WasteRecord persistence — maps to `waste_records` table
// ---------------------------------------------------------------------------

void SqliteFinanceRepository::saveWasteRecord(const WasteRecord& record) {
    const char* sql =
        "INSERT INTO waste_records (id, item_id, quantity, unit, reason, recorded_at) "
        "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("saveWasteRecord prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, record.id().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.costItemId().value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, record.quantity());
    sqlite3_bind_text(stmt, 4, record.unit().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, record.reason().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, record.recordedAt().c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("saveWasteRecord step: " + err);
    }
    sqlite3_finalize(stmt);
}

std::vector<WasteRecord> SqliteFinanceRepository::findAllWasteRecords() {
    const char* sql =
        "SELECT id, item_id, quantity, unit, reason, recorded_at FROM waste_records;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findAllWasteRecords prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    std::vector<WasteRecord> records;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
            sqlite3_column_double(stmt, 2),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
        );
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<WasteRecord> SqliteFinanceRepository::findWasteRecordsByDateRange(
    const std::string& start, const std::string& end) {
    const char* sql =
        "SELECT id, item_id, quantity, unit, reason, recorded_at "
        "FROM waste_records WHERE recorded_at >= ? AND recorded_at <= ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("findWasteRecordsByDateRange prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, start.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, end.c_str(), -1, SQLITE_TRANSIENT);
    std::vector<WasteRecord> records;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back(
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))},
            shared::Id{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
            sqlite3_column_double(stmt, 2),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
        );
    }
    sqlite3_finalize(stmt);
    return records;
}

// ---------------------------------------------------------------------------
// Food cost ratio — sum ingredient costs vs sum selling prices
// ---------------------------------------------------------------------------

FoodCostRatio SqliteFinanceRepository::calculateFoodCostRatio() {
    const char* sql =
        "SELECT COALESCE(SUM(ingredient_cost_cents), 0), COALESCE(SUM(price_cents), 0) FROM items;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("calculateFoodCostRatio prepare: ") + sqlite3_errmsg(db_.handle()));
    }
    int ingredientCost = 0;
    int revenue = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ingredientCost = sqlite3_column_int(stmt, 0);
        revenue = sqlite3_column_int(stmt, 1);
    }
    sqlite3_finalize(stmt);
    return FoodCostRatio(shared::Money(ingredientCost), shared::Money(revenue));
}

} // namespace finance
