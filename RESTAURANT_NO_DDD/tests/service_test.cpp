#include <catch2/catch_test_macros.hpp>
#include "db.h"
#include "repositories/item_repository.h"
#include "repositories/order_repository.h"
#include "repositories/table_repository.h"
#include "repositories/reservation_repository.h"
#include "services/item_service.h"
#include "services/order_service.h"
#include "services/table_service.h"
#include "services/reservation_service.h"

// Each service test uses its own isolated in-memory DB.

static void setupAllTables(Database& db) {
    db.exec("PRAGMA foreign_keys=OFF;");
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
    db.exec(
        "CREATE TABLE IF NOT EXISTS orders ("
        "  id TEXT PRIMARY KEY,"
        "  table_number INTEGER NOT NULL,"
        "  status TEXT NOT NULL DEFAULT 'new',"
        "  created_at TEXT NOT NULL"
        ");"
    );
    db.exec(
        "CREATE TABLE IF NOT EXISTS order_lines ("
        "  id TEXT PRIMARY KEY,"
        "  order_id TEXT NOT NULL,"
        "  item_id TEXT NOT NULL,"
        "  status TEXT NOT NULL DEFAULT 'waiting',"
        "  fire_at_offset_minutes INTEGER,"
        "  fired_at TEXT,"
        "  plated_at TEXT"
        ");"
    );
    db.exec(
        "CREATE TABLE IF NOT EXISTS tables ("
        "  id TEXT PRIMARY KEY,"
        "  table_number INTEGER NOT NULL,"
        "  capacity INTEGER NOT NULL,"
        "  status TEXT NOT NULL DEFAULT 'available'"
        ");"
    );
    db.exec(
        "CREATE TABLE IF NOT EXISTS seatings ("
        "  id TEXT PRIMARY KEY,"
        "  table_id TEXT NOT NULL,"
        "  cover_count INTEGER NOT NULL DEFAULT 0,"
        "  is_walk_in INTEGER NOT NULL DEFAULT 0,"
        "  reservation_name TEXT,"
        "  seated_at TEXT,"
        "  cleared_at TEXT"
        ");"
    );
}

// ---------------------------------------------------------------------------
// ItemService tests
// ---------------------------------------------------------------------------

TEST_CASE("ItemService creates item with generated id") {
    Database db(":memory:");
    setupAllTables(db);
    ItemRepository repo(db);
    ItemService svc(repo);

    std::string id = svc.createItem("Grilled Salmon", "Atlantic salmon", 2800, 15,
                                    "GRILL", "FISH", 900, 700);

    REQUIRE_FALSE(id.empty());
    REQUIRE(id.size() == 32);

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->name == "Grilled Salmon");
    REQUIRE(found->price_cents == 2800);
    REQUIRE(found->is_available == true);
}

TEST_CASE("ItemService update item availability") {
    Database db(":memory:");
    setupAllTables(db);
    ItemRepository repo(db);
    ItemService svc(repo);

    std::string id = svc.createItem("Caesar Salad", "", 1400, 5, "COLD", "SALAD", 300, 200);
    REQUIRE_FALSE(id.empty());

    svc.updateAvailability(id, false);
    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->is_available == false);

    svc.updateAvailability(id, true);
    found = repo.findById(id);
    REQUIRE(found->is_available == true);
}

// ---------------------------------------------------------------------------
// OrderService tests
// ---------------------------------------------------------------------------

TEST_CASE("OrderService creates order with lines") {
    Database db(":memory:");
    setupAllTables(db);
    OrderRepository repo(db);
    OrderService svc(repo);

    std::vector<std::string> item_ids = {"item-a", "item-b"};
    std::string id = svc.createOrder(3, item_ids);

    REQUIRE_FALSE(id.empty());
    REQUIRE(id.size() == 32);

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->order.table_number == 3);
    REQUIRE(found->order.status == "OPEN");
    REQUIRE(found->lines.size() == 2);
    REQUIRE_FALSE(found->order.created_at.empty());
}

TEST_CASE("OrderService update order status to in_progress") {
    Database db(":memory:");
    setupAllTables(db);
    OrderRepository repo(db);
    OrderService svc(repo);

    std::string id = svc.createOrder(7, {});
    REQUIRE_FALSE(id.empty());

    svc.updateOrderStatus(id, "IN_PROGRESS");

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->order.status == "IN_PROGRESS");
}

// ---------------------------------------------------------------------------
// TableService tests
// ---------------------------------------------------------------------------

TEST_CASE("TableService creates table") {
    Database db(":memory:");
    setupAllTables(db);
    TableRepository repo(db);
    TableService svc(repo);

    std::string id = svc.createTable(5, 4);

    REQUIRE_FALSE(id.empty());
    REQUIRE(id.size() == 32);

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->table_number == 5);
    REQUIRE(found->capacity == 4);
    REQUIRE(found->status == "AVAILABLE");
}

TEST_CASE("TableService update table status") {
    Database db(":memory:");
    setupAllTables(db);
    TableRepository repo(db);
    TableService svc(repo);

    std::string id = svc.createTable(8, 6);
    REQUIRE_FALSE(id.empty());

    svc.updateTableStatus(id, "OCCUPIED");

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->status == "OCCUPIED");
}

// ---------------------------------------------------------------------------
// ReservationService tests
// ---------------------------------------------------------------------------

TEST_CASE("ReservationService create reservation") {
    Database db(":memory:");
    setupAllTables(db);
    ReservationRepository repo(db);
    ReservationService svc(repo);

    std::string id = svc.createReservation("table-xyz", 4, "RESERVED", "Smith");

    REQUIRE_FALSE(id.empty());
    REQUIRE(id.size() == 32);

    auto found = repo.findById(id);
    REQUIRE(found.has_value());
    REQUIRE(found->table_id == "table-xyz");
    REQUIRE(found->guest_count == 4);
    REQUIRE(found->type == "RESERVED");
    REQUIRE(found->reservation_name == "Smith");
    REQUIRE_FALSE(found->seated_at.empty());
}
