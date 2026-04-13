#include "services/item_service.h"
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

ItemService::ItemService(ItemRepository& repo) : repo_(repo) {}

std::string ItemService::generateId() {
    return generateHexId();
}

std::string ItemService::createItem(
    const std::string& name,
    const std::string& description,
    int price_cents,
    int prep_time_minutes,
    const std::string& cooking_method,
    const std::string& station,
    int ingredient_cost_cents,
    int supplier_price_cents)
{
    Item item;
    item.id                    = generateId();
    item.name                  = name;
    item.description           = description;
    item.price_cents           = price_cents;
    item.prep_time_minutes     = prep_time_minutes;
    item.cooking_method        = cooking_method;
    item.station               = station;
    item.ingredient_cost_cents = ingredient_cost_cents;
    item.supplier_price_cents  = supplier_price_cents;
    item.is_available          = true;
    repo_.create(item);
    return item.id;
}

std::vector<Item> ItemService::getItems() {
    return repo_.findAll();
}

std::optional<Item> ItemService::getItem(const std::string& id) {
    return repo_.findById(id);
}

void ItemService::updateItem(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    int price_cents,
    int prep_time_minutes,
    const std::string& cooking_method,
    const std::string& station,
    int ingredient_cost_cents,
    int supplier_price_cents)
{
    Item item;
    item.id                    = id;
    item.name                  = name;
    item.description           = description;
    item.price_cents           = price_cents;
    item.prep_time_minutes     = prep_time_minutes;
    item.cooking_method        = cooking_method;
    item.station               = station;
    item.ingredient_cost_cents = ingredient_cost_cents;
    item.supplier_price_cents  = supplier_price_cents;
    item.is_available          = true; // preserved by repo update
    repo_.update(item);
}

void ItemService::updateAvailability(const std::string& id, bool available) {
    repo_.updateAvailability(id, available);
}
