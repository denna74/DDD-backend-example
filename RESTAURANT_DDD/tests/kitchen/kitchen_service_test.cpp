#include <catch2/catch_test_macros.hpp>
#include "kitchen/application/kitchen_service.h"
#include "kitchen/adapters/sqlite_kitchen_repository.h"
#include "kitchen/ports/i_kitchen_event_publisher.h"
#include "shared_kernel/event_bus.h"
#include "shared_kernel/db.h"

namespace {

// Test event publisher that wraps shared::EventBus
class TestEventPublisher : public kitchen::IKitchenEventPublisher {
public:
    explicit TestEventPublisher(shared::EventBus& bus) : bus_(bus) {}

    void publishDishFired(const kitchen::DishFired& e) override {
        bus_.publish(e);
    }
    void publishDishPlated(const kitchen::DishPlated& e) override {
        bus_.publish(e);
    }
    void publishAllDishesPlated(const kitchen::AllDishesPlated& e) override {
        bus_.publish(e);
    }
    void publishDishMarkedOutOfStock(const kitchen::DishMarkedOutOfStock& e) override {
        bus_.publish(e);
    }

private:
    shared::EventBus& bus_;
};

struct TestFixture {
    Database db{":memory:"};
    shared::EventBus eventBus;
    kitchen::SqliteKitchenRepository repo{db};
    TestEventPublisher publisher{eventBus};
    kitchen::KitchenService service{repo, publisher};

    TestFixture() {
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
    }
};

} // anonymous namespace

TEST_CASE("KitchenService add dish and list dishes") {
    TestFixture f;

    auto id = f.service.addDish("Salmon", 12, "pan-sear", "grill");
    REQUIRE_FALSE(id.empty());

    auto dishes = f.service.listDishes();
    REQUIRE(dishes.size() == 1);
    REQUIRE(dishes[0].name() == "Salmon");
    REQUIRE(dishes[0].prepTimeMinutes() == 12);
    REQUIRE(dishes[0].cookingMethod() == "pan-sear");
    REQUIRE(dishes[0].station() == kitchen::Station::Grill);
    REQUIRE(dishes[0].isAvailable());
}

TEST_CASE("KitchenService mark dish out of stock publishes event") {
    TestFixture f;

    auto id = f.service.addDish("Risotto", 22, "simmer", "sauce");

    bool eventReceived = false;
    std::string eventDishName;
    f.eventBus.subscribe<kitchen::DishMarkedOutOfStock>(
        [&](const kitchen::DishMarkedOutOfStock& e) {
            eventReceived = true;
            eventDishName = e.dishName;
        });

    auto result = f.service.markDishOutOfStock(id);
    REQUIRE(result.isOk());
    REQUIRE(eventReceived);
    REQUIRE(eventDishName == "Risotto");

    auto dish = f.service.getDish(id);
    REQUIRE(dish.has_value());
    REQUIRE_FALSE(dish->isAvailable());
}

TEST_CASE("KitchenService create and execute fire order") {
    TestFixture f;

    // Create dishes first
    auto dishId1 = f.service.addDish("Salmon", 12, "pan-sear", "grill");
    auto dishId2 = f.service.addDish("Fondant", 14, "bake", "pastry");

    // Track events
    bool dishFiredEvent = false;
    bool dishPlatedEvent = false;
    bool allPlatedEvent = false;
    int allPlatedTable = -1;

    f.eventBus.subscribe<kitchen::DishFired>(
        [&](const kitchen::DishFired&) { dishFiredEvent = true; });
    f.eventBus.subscribe<kitchen::DishPlated>(
        [&](const kitchen::DishPlated&) { dishPlatedEvent = true; });
    f.eventBus.subscribe<kitchen::AllDishesPlated>(
        [&](const kitchen::AllDishesPlated& e) {
            allPlatedEvent = true;
            allPlatedTable = e.tableNumber;
        });

    // Create fire order
    kitchen::CreateFireOrderRequest request;
    request.tableNumber = 5;
    request.dishes = {
        {dishId1, 0},
        {dishId2, 5}
    };
    auto orderId = f.service.createFireOrder(request);
    REQUIRE_FALSE(orderId.empty());

    // Verify order was created
    auto order = f.service.getFireOrder(orderId);
    REQUIRE(order.has_value());
    REQUIRE(order->tableNumber() == 5);
    REQUIRE(order->lines().size() == 2);
    REQUIRE(order->status() == kitchen::FireOrderStatus::Coordinating);

    auto lineId1 = order->lines()[0].id().value;
    auto lineId2 = order->lines()[1].id().value;

    // Fire first dish
    auto fireResult = f.service.fireDish(orderId, lineId1);
    REQUIRE(fireResult.isOk());
    REQUIRE(dishFiredEvent);

    // Fire second dish
    f.service.fireDish(orderId, lineId2);

    // Plate first dish
    auto plateResult = f.service.plateDish(orderId, lineId1);
    REQUIRE(plateResult.isOk());
    REQUIRE(dishPlatedEvent);
    REQUIRE_FALSE(allPlatedEvent);

    // Plate second dish -- should trigger AllDishesPlated
    f.service.plateDish(orderId, lineId2);
    REQUIRE(allPlatedEvent);
    REQUIRE(allPlatedTable == 5);

    // Verify final state
    auto finalOrder = f.service.getFireOrder(orderId);
    REQUIRE(finalOrder.has_value());
    REQUIRE(finalOrder->status() == kitchen::FireOrderStatus::Plated);
}
