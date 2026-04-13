#include <catch2/catch_test_macros.hpp>
#include "db.h"
#include "repositories/order_repository.h"

static Database& testDb() {
    static Database db(":memory:");
    static bool initialized = false;
    if (!initialized) {
        db.exec(
            "CREATE TABLE IF NOT EXISTS orders ("
            "  id TEXT PRIMARY KEY,"
            "  table_number INTEGER NOT NULL,"
            "  status TEXT NOT NULL,"
            "  created_at TEXT"
            ");"
        );
        db.exec(
            "CREATE TABLE IF NOT EXISTS order_lines ("
            "  id TEXT PRIMARY KEY,"
            "  order_id TEXT NOT NULL,"
            "  item_id TEXT NOT NULL,"
            "  status TEXT NOT NULL,"
            "  fire_at_offset_minutes INTEGER,"
            "  fired_at TEXT,"
            "  plated_at TEXT"
            ");"
        );
        initialized = true;
    }
    return db;
}

TEST_CASE("Order: create with lines and findById") {
    auto& db = testDb();
    OrderRepository repo(db);

    Order order;
    order.id           = "order-001";
    order.table_number = 5;
    order.status       = "OPEN";
    order.created_at   = "2026-04-10T18:00:00";

    OrderLine line1;
    line1.id       = "line-001";
    line1.order_id = "order-001";
    line1.item_id  = "item-001";
    line1.status   = "PENDING";

    OrderLine line2;
    line2.id                    = "line-002";
    line2.order_id              = "order-001";
    line2.item_id               = "item-002";
    line2.status                = "PENDING";
    line2.fire_at_offset_minutes = 10;

    repo.create(order, {line1, line2});

    auto found = repo.findById("order-001");
    REQUIRE(found.has_value());
    REQUIRE(found->order.id == "order-001");
    REQUIRE(found->order.table_number == 5);
    REQUIRE(found->order.status == "OPEN");
    REQUIRE(found->order.created_at == "2026-04-10T18:00:00");
    REQUIRE(found->lines.size() == 2);

    bool hasLine2 = false;
    for (const auto& l : found->lines) {
        if (l.id == "line-002") {
            hasLine2 = true;
            REQUIRE(l.fire_at_offset_minutes.has_value());
            REQUIRE(l.fire_at_offset_minutes.value() == 10);
        }
    }
    REQUIRE(hasLine2);
}

TEST_CASE("Order: findAll returns all orders") {
    auto& db = testDb();
    OrderRepository repo(db);

    Order order1;
    order1.id           = "fa-order-001";
    order1.table_number = 1;
    order1.status       = "OPEN";
    repo.create(order1, {});

    Order order2;
    order2.id           = "fa-order-002";
    order2.table_number = 3;
    order2.status       = "OPEN";
    repo.create(order2, {});

    auto all = repo.findAll();
    REQUIRE(all.size() >= 2);
}

TEST_CASE("Order: updateStatus") {
    auto& db = testDb();
    OrderRepository repo(db);

    Order order;
    order.id           = "order-003";
    order.table_number = 7;
    order.status       = "OPEN";
    repo.create(order, {});

    repo.updateStatus("order-003", "CLOSED");

    auto found = repo.findById("order-003");
    REQUIRE(found.has_value());
    REQUIRE(found->order.status == "CLOSED");
}
