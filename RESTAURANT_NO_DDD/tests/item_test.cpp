#include <catch2/catch_test_macros.hpp>
#include "db.h"
#include "repositories/item_repository.h"

static Database& testDb() {
    static Database db(":memory:");
    static bool initialized = false;
    if (!initialized) {
        db.exec(
            "CREATE TABLE IF NOT EXISTS items ("
            "  id TEXT PRIMARY KEY,"
            "  name TEXT NOT NULL,"
            "  description TEXT,"
            "  price_cents INTEGER NOT NULL DEFAULT 0,"
            "  prep_time_minutes INTEGER NOT NULL DEFAULT 0,"
            "  cooking_method TEXT NOT NULL DEFAULT '',"
            "  station TEXT NOT NULL DEFAULT '',"
            "  ingredient_cost_cents INTEGER NOT NULL DEFAULT 0,"
            "  supplier_price_cents INTEGER NOT NULL DEFAULT 0,"
            "  is_available INTEGER NOT NULL DEFAULT 1"
            ");"
        );
        initialized = true;
    }
    return db;
}

TEST_CASE("Item: create and findById") {
    auto& db = testDb();
    ItemRepository repo(db);

    Item item;
    item.id                   = "item-001";
    item.name                 = "Grilled Salmon";
    item.description          = "Atlantic salmon, grilled";
    item.price_cents          = 2800;
    item.prep_time_minutes    = 15;
    item.cooking_method       = "GRILL";
    item.station              = "FISH";
    item.ingredient_cost_cents = 900;
    item.supplier_price_cents  = 700;
    item.is_available         = true;

    repo.create(item);

    auto found = repo.findById("item-001");
    REQUIRE(found.has_value());
    REQUIRE(found->id == "item-001");
    REQUIRE(found->name == "Grilled Salmon");
    REQUIRE(found->description == "Atlantic salmon, grilled");
    REQUIRE(found->price_cents == 2800);
    REQUIRE(found->prep_time_minutes == 15);
    REQUIRE(found->cooking_method == "GRILL");
    REQUIRE(found->station == "FISH");
    REQUIRE(found->ingredient_cost_cents == 900);
    REQUIRE(found->supplier_price_cents == 700);
    REQUIRE(found->is_available == true);
}

TEST_CASE("Item: findAll returns all items") {
    auto& db = testDb();
    ItemRepository repo(db);

    Item item1;
    item1.id             = "fa-item-001";
    item1.name           = "Grilled Salmon";
    item1.cooking_method = "GRILL";
    item1.station        = "FISH";
    repo.create(item1);

    Item item2;
    item2.id             = "fa-item-002";
    item2.name           = "Caesar Salad";
    item2.price_cents    = 1400;
    item2.cooking_method = "COLD";
    item2.station        = "SALAD";
    repo.create(item2);

    auto all = repo.findAll();
    REQUIRE(all.size() >= 2);
}

TEST_CASE("Item: update") {
    auto& db = testDb();
    ItemRepository repo(db);

    Item item;
    item.id             = "item-003";
    item.name           = "Pasta";
    item.price_cents    = 1800;
    item.cooking_method = "BOIL";
    item.station        = "PASTA";
    repo.create(item);

    item.name        = "Truffle Pasta";
    item.price_cents = 2400;
    repo.update(item);

    auto found = repo.findById("item-003");
    REQUIRE(found.has_value());
    REQUIRE(found->name == "Truffle Pasta");
    REQUIRE(found->price_cents == 2400);
}

TEST_CASE("Item: updateAvailability") {
    auto& db = testDb();
    ItemRepository repo(db);

    Item item;
    item.id             = "item-004";
    item.name           = "Soup of the Day";
    item.cooking_method = "SIMMER";
    item.station        = "SOUP";
    item.is_available   = true;
    repo.create(item);

    repo.updateAvailability("item-004", false);

    auto found = repo.findById("item-004");
    REQUIRE(found.has_value());
    REQUIRE(found->is_available == false);

    repo.updateAvailability("item-004", true);
    found = repo.findById("item-004");
    REQUIRE(found->is_available == true);
}
