#include "services/table_service.h"
#include <random>
#include <sstream>
#include <iomanip>

static std::string generateHexId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << dist(gen)
        << std::setw(16) << dist(gen);
    return oss.str();
}

TableService::TableService(TableRepository& repo) : repo_(repo) {}

std::string TableService::generateId() { return generateHexId(); }

std::string TableService::createTable(int table_number, int capacity) {
    Table table;
    table.id           = generateId();
    table.table_number = table_number;
    table.capacity     = capacity;
    table.status       = "AVAILABLE";
    repo_.create(table);
    return table.id;
}

std::vector<Table> TableService::getTables() {
    return repo_.findAll();
}

std::optional<Table> TableService::getTable(const std::string& id) {
    return repo_.findById(id);
}

void TableService::updateTableStatus(const std::string& id, const std::string& status) {
    repo_.updateStatus(id, status);
}
