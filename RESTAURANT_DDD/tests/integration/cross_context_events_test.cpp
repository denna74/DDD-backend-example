#include <catch2/catch_test_macros.hpp>

#include "shared_kernel/db.h"
#include "shared_kernel/event_bus.h"

#include "kitchen/ports/i_kitchen_event_publisher.h"
#include "kitchen/adapters/sqlite_kitchen_repository.h"
#include "kitchen/application/kitchen_service.h"

#include "floor/ports/i_floor_event_publisher.h"
#include "floor/adapters/sqlite_floor_repository.h"
#include "floor/application/floor_service.h"

#include "finance/ports/i_finance_event_publisher.h"
#include "finance/adapters/sqlite_finance_repository.h"
#include "finance/application/finance_service.h"

namespace {

// EventBus publisher adapters (same pattern as main.cpp)

struct KitchenEventBusPublisher : kitchen::IKitchenEventPublisher {
    explicit KitchenEventBusPublisher(shared::EventBus& bus) : bus_(bus) {}
    void publishDishFired(const kitchen::DishFired& e) override { bus_.publish(e); }
    void publishDishPlated(const kitchen::DishPlated& e) override { bus_.publish(e); }
    void publishAllDishesPlated(const kitchen::AllDishesPlated& e) override { bus_.publish(e); }
    void publishDishMarkedOutOfStock(const kitchen::DishMarkedOutOfStock& e) override { bus_.publish(e); }
private:
    shared::EventBus& bus_;
};

struct FloorEventBusPublisher : floor_ctx::IFloorEventPublisher {
    explicit FloorEventBusPublisher(shared::EventBus& bus) : bus_(bus) {}
    void publishWalkInSeated(const floor_ctx::WalkInSeated& e) override { bus_.publish(e); }
    void publishReservationSeated(const floor_ctx::ReservationSeated& e) override { bus_.publish(e); }
    void publishTableTurned(const floor_ctx::TableTurned& e) override { bus_.publish(e); }
    void publishMenuItemSoldOut(const floor_ctx::MenuItemSoldOut& e) override { bus_.publish(e); }
private:
    shared::EventBus& bus_;
};

struct FinanceEventBusPublisher : finance::IFinanceEventPublisher {
    explicit FinanceEventBusPublisher(shared::EventBus& bus) : bus_(bus) {}
    void publishWasteRecorded(const finance::WasteRecorded& e) override { bus_.publish(e); }
private:
    shared::EventBus& bus_;
};

// Full cross-context fixture: in-memory DB with all tables, all three contexts wired.
struct CrossContextFixture {
    Database db{":memory:"};
    shared::EventBus eventBus;

    // Publishers
    KitchenEventBusPublisher kitchenPublisher{eventBus};
    FloorEventBusPublisher   floorPublisher{eventBus};
    FinanceEventBusPublisher financePublisher{eventBus};

    // Repositories
    kitchen::SqliteKitchenRepository kitchenRepo{db};
    floor_ctx::SqliteFloorRepository floorRepo{db};
    finance::SqliteFinanceRepository financeRepo{db};

    // Services
    kitchen::KitchenService kitchenService{kitchenRepo, kitchenPublisher};
    floor_ctx::FloorService floorService{floorRepo, floorPublisher};
    finance::FinanceService financeService{financeRepo, financePublisher};

    CrossContextFixture() {
        // Create ALL tables (shared schema)
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

        // Wire cross-context event: Kitchen DishMarkedOutOfStock -> Floor markSoldOutByName
        eventBus.subscribe<kitchen::DishMarkedOutOfStock>(
            [this](const kitchen::DishMarkedOutOfStock& e) {
                floorService.markSoldOutByName(e.dishName);
            });
    }
};

} // anonymous namespace

TEST_CASE("Kitchen marks dish out of stock -> Floor marks menu item sold out") {
    CrossContextFixture f;

    // 1. Kitchen adds a dish (inserts into shared `items` table).
    //    Kitchen's saveDish fills: name, prep_time_minutes, cooking_method, station,
    //    and sets price_cents=0, description='', costs=0.
    auto dishId = f.kitchenService.addDish("Salmon", 12, "pan-sear", "grill");
    REQUIRE_FALSE(dishId.empty());

    // 2. Verify Floor can see this item as a menu item (same DB row).
    auto menuItems = f.floorService.listMenuItems();
    REQUIRE(menuItems.size() == 1);
    REQUIRE(menuItems[0].name() == "Salmon");
    REQUIRE(menuItems[0].isAvailable());

    // 3. Kitchen marks the dish out of stock.
    //    This publishes DishMarkedOutOfStock on the EventBus.
    //    The cross-context subscription calls floorService.markSoldOutByName("Salmon").
    auto result = f.kitchenService.markDishOutOfStock(dishId);
    REQUIRE(result.isOk());

    // 4. Verify the Kitchen side: dish is out of stock.
    auto dish = f.kitchenService.getDish(dishId);
    REQUIRE(dish.has_value());
    REQUIRE_FALSE(dish->isAvailable());

    // 5. Verify the Floor side: menu item is now sold out.
    auto updatedMenuItems = f.floorService.listMenuItems();
    REQUIRE(updatedMenuItems.size() == 1);
    REQUIRE_FALSE(updatedMenuItems[0].isAvailable());
}
