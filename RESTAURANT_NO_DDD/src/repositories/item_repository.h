#pragma once
#include "models/item.h"
#include "db.h"
#include <vector>
#include <optional>

class ItemRepository {
public:
    explicit ItemRepository(Database& db) : db_(db) {}
    void create(const Item& item);
    std::optional<Item> findById(const std::string& id);
    std::vector<Item> findAll();
    void update(const Item& item);
    void updateAvailability(const std::string& id, bool available);
private:
    Database& db_;
    static Item rowToItem(sqlite3_stmt* stmt);
};
