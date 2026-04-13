#pragma once
#include "repositories/item_repository.h"
#include <string>
#include <vector>
#include <optional>

class ItemService {
public:
    explicit ItemService(ItemRepository& repo);

    std::string createItem(
        const std::string& name,
        const std::string& description,
        int price_cents,
        int prep_time_minutes,
        const std::string& cooking_method,
        const std::string& station,
        int ingredient_cost_cents,
        int supplier_price_cents
    );

    std::vector<Item> getItems();
    std::optional<Item> getItem(const std::string& id);

    void updateItem(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        int price_cents,
        int prep_time_minutes,
        const std::string& cooking_method,
        const std::string& station,
        int ingredient_cost_cents,
        int supplier_price_cents
    );

    void updateAvailability(const std::string& id, bool available);

private:
    ItemRepository& repo_;
    static std::string generateId();
};
