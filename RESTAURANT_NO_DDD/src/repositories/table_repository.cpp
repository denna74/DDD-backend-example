#include "repositories/table_repository.h"
#include <stdexcept>

Table TableRepository::rowToTable(sqlite3_stmt* stmt) {
    Table table;
    table.id           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    table.table_number = sqlite3_column_int(stmt, 1);
    table.capacity     = sqlite3_column_int(stmt, 2);
    table.status       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    return table;
}

void TableRepository::create(const Table& table) {
    const char* sql =
        "INSERT INTO tables (id, table_number, capacity, status) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("TableRepository::create prepare failed");
    }
    sqlite3_bind_text(stmt, 1, table.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, table.table_number);
    sqlite3_bind_int(stmt, 3, table.capacity);
    sqlite3_bind_text(stmt, 4, table.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<Table> TableRepository::findById(const std::string& id) {
    const char* sql =
        "SELECT id, table_number, capacity, status FROM tables WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("TableRepository::findById prepare failed");
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    std::optional<Table> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToTable(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Table> TableRepository::findAll() {
    const char* sql = "SELECT id, table_number, capacity, status FROM tables;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("TableRepository::findAll prepare failed");
    }
    std::vector<Table> tables;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tables.push_back(rowToTable(stmt));
    }
    sqlite3_finalize(stmt);
    return tables;
}

void TableRepository::updateStatus(const std::string& id, const std::string& status) {
    const char* sql = "UPDATE tables SET status=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("TableRepository::updateStatus prepare failed");
    }
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
