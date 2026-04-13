#pragma once
#include "models/table.h"
#include "db.h"
#include <vector>
#include <optional>

class TableRepository {
public:
    explicit TableRepository(Database& db) : db_(db) {}
    void create(const Table& table);
    std::optional<Table> findById(const std::string& id);
    std::vector<Table> findAll();
    void updateStatus(const std::string& id, const std::string& status);
private:
    Database& db_;
    static Table rowToTable(sqlite3_stmt* stmt);
};
