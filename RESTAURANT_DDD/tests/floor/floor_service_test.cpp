#include <catch2/catch_test_macros.hpp>
#include "floor/application/floor_service.h"
#include "floor/adapters/sqlite_floor_repository.h"
#include "floor/ports/i_floor_event_publisher.h"
#include "shared_kernel/event_bus.h"
#include "shared_kernel/db.h"

namespace {

// Test event publisher that wraps shared::EventBus
class TestFloorEventPublisher : public floor_ctx::IFloorEventPublisher {
public:
    explicit TestFloorEventPublisher(shared::EventBus& bus) : bus_(bus) {}

    void publishWalkInSeated(const floor_ctx::WalkInSeated& e) override {
        bus_.publish(e);
    }
    void publishReservationSeated(const floor_ctx::ReservationSeated& e) override {
        bus_.publish(e);
    }
    void publishTableTurned(const floor_ctx::TableTurned& e) override {
        bus_.publish(e);
    }
    void publishMenuItemSoldOut(const floor_ctx::MenuItemSoldOut& e) override {
        bus_.publish(e);
    }

private:
    shared::EventBus& bus_;
};

struct FloorTestFixture {
    Database db{":memory:"};
    shared::EventBus eventBus;
    floor_ctx::SqliteFloorRepository repo{db};
    TestFloorEventPublisher publisher{eventBus};
    floor_ctx::FloorService service{repo, publisher};

    FloorTestFixture() {
        db.exec(
            "CREATE TABLE IF NOT EXISTS items ("
            "  id TEXT PRIMARY KEY,"
            "  name TEXT NOT NULL,"
            "  description TEXT,"
            "  price_cents INTEGER NOT NULL,"
            "  prep_time_minutes INTEGER NOT NULL,"
            "  cooking_method TEXT NOT NULL,"
            "  station TEXT NOT NULL,"
            "  ingredient_cost_cents INTEGER NOT NULL,"
            "  supplier_price_cents INTEGER NOT NULL,"
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
            "  order_id TEXT NOT NULL REFERENCES orders(id),"
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
            "  table_number INTEGER NOT NULL UNIQUE,"
            "  capacity INTEGER NOT NULL,"
            "  status TEXT NOT NULL DEFAULT 'available'"
            ");"
        );
        db.exec(
            "CREATE TABLE IF NOT EXISTS seatings ("
            "  id TEXT PRIMARY KEY,"
            "  table_id TEXT NOT NULL REFERENCES tables(id),"
            "  cover_count INTEGER NOT NULL,"
            "  is_walk_in INTEGER NOT NULL DEFAULT 0,"
            "  reservation_name TEXT,"
            "  seated_at TEXT NOT NULL,"
            "  cleared_at TEXT"
            ");"
        );
        db.exec(
            "CREATE TABLE IF NOT EXISTS waste_records ("
            "  id TEXT PRIMARY KEY,"
            "  item_id TEXT NOT NULL REFERENCES items(id),"
            "  quantity REAL NOT NULL,"
            "  unit TEXT NOT NULL,"
            "  reason TEXT NOT NULL,"
            "  recorded_at TEXT NOT NULL"
            ");"
        );
    }
};

} // anonymous namespace

TEST_CASE("FloorService seat walk-in marks table occupied") {
    FloorTestFixture f;

    auto tableId = f.service.addTable(1, 4);
    REQUIRE_FALSE(tableId.empty());

    bool eventReceived = false;
    int eventCovers = 0;
    f.eventBus.subscribe<floor_ctx::WalkInSeated>(
        [&](const floor_ctx::WalkInSeated& e) {
            eventReceived = true;
            eventCovers = e.coverCount;
        });

    auto result = f.service.seatWalkIn(tableId, 3);
    REQUIRE(result.isOk());
    REQUIRE(eventReceived);
    REQUIRE(eventCovers == 3);

    auto tables = f.service.listTables();
    REQUIRE(tables.size() == 1);
    REQUIRE(tables[0].status() == floor_ctx::TableStatus::Occupied);
}

TEST_CASE("FloorService cannot seat at occupied table") {
    FloorTestFixture f;

    auto tableId = f.service.addTable(2, 6);
    f.service.seatWalkIn(tableId, 2);

    auto result = f.service.seatWalkIn(tableId, 4);
    REQUIRE(result.isError());
    REQUIRE(result.error == "Table is not available or reserved");
}

TEST_CASE("FloorService turn table publishes event") {
    FloorTestFixture f;

    auto tableId = f.service.addTable(3, 4);
    f.service.seatWalkIn(tableId, 2);

    bool turnEventReceived = false;
    int turnCoverCount = 0;
    f.eventBus.subscribe<floor_ctx::TableTurned>(
        [&](const floor_ctx::TableTurned& e) {
            turnEventReceived = true;
            turnCoverCount = e.coverCount;
        });

    auto result = f.service.turnTable(tableId);
    REQUIRE(result.isOk());
    REQUIRE(turnEventReceived);
    REQUIRE(turnCoverCount == 2);

    auto tables = f.service.listTables();
    REQUIRE(tables[0].status() == floor_ctx::TableStatus::Available);
}

TEST_CASE("FloorService mark menu item sold out publishes event") {
    FloorTestFixture f;

    auto itemId = f.service.addMenuItem("Salmon", "Pan-seared salmon", 2800);

    bool eventReceived = false;
    std::string eventName;
    f.eventBus.subscribe<floor_ctx::MenuItemSoldOut>(
        [&](const floor_ctx::MenuItemSoldOut& e) {
            eventReceived = true;
            eventName = e.name;
        });

    auto result = f.service.markSoldOut(itemId);
    REQUIRE(result.isOk());
    REQUIRE(eventReceived);
    REQUIRE(eventName == "Salmon");

    auto items = f.service.listMenuItems();
    REQUIRE(items.size() == 1);
    REQUIRE_FALSE(items[0].isAvailable());
}

TEST_CASE("FloorService count covers tonight") {
    FloorTestFixture f;

    auto tableId1 = f.service.addTable(1, 4);
    auto tableId2 = f.service.addTable(2, 6);

    f.service.seatWalkIn(tableId1, 3);
    f.service.seatWalkIn(tableId2, 5);

    // Both seatings are from today, so covers = 3 + 5 = 8
    int covers = f.service.countCoversTonight();
    REQUIRE(covers == 8);
}
