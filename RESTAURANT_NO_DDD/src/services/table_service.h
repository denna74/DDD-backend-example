#pragma once
#include "repositories/table_repository.h"
#include <string>
#include <vector>
#include <optional>

class TableService {
public:
    explicit TableService(TableRepository& repo);

    std::string createTable(int table_number, int capacity);
    std::vector<Table> getTables();
    std::optional<Table> getTable(const std::string& id);
    void updateTableStatus(const std::string& id, const std::string& status);

private:
    TableRepository& repo_;
    static std::string generateId();
};
