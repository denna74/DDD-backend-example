#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "finance/application/finance_service.h"
#include "finance/adapters/sqlite_finance_repository.h"
#include "finance/ports/i_finance_event_publisher.h"
#include "shared_kernel/event_bus.h"
#include "shared_kernel/db.h"

namespace {

class TestFinanceEventPublisher : public finance::IFinanceEventPublisher {
public:
    explicit TestFinanceEventPublisher(shared::EventBus& bus) : bus_(bus) {}

    void publishWasteRecorded(const finance::WasteRecorded& e) override {
        bus_.publish(e);
    }

private:
    shared::EventBus& bus_;
};

struct FinanceTestFixture {
    Database db{":memory:"};
    shared::EventBus eventBus;
    finance::SqliteFinanceRepository repo{db};
    TestFinanceEventPublisher publisher{eventBus};
    finance::FinanceService service{repo, publisher};

    FinanceTestFixture() {
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

TEST_CASE("FinanceService add cost item and list") {
    FinanceTestFixture f;

    auto id = f.service.addCostItem("Salmon", 850, 900, 2800);
    REQUIRE_FALSE(id.empty());

    auto items = f.service.listCostItems();
    REQUIRE(items.size() == 1);
    REQUIRE(items[0].name() == "Salmon");
    REQUIRE(items[0].ingredientCost().cents == 850);
    REQUIRE(items[0].supplierPrice().cents == 900);
    REQUIRE(items[0].sellingPrice().cents == 2800);
}

TEST_CASE("FinanceService record waste publishes event") {
    FinanceTestFixture f;

    auto itemId = f.service.addCostItem("Salmon", 850, 900, 2800);

    bool eventReceived = false;
    std::string eventReason;
    f.eventBus.subscribe<finance::WasteRecorded>(
        [&](const finance::WasteRecorded& e) {
            eventReceived = true;
            eventReason = e.reason;
        });

    auto wasteId = f.service.recordWaste(itemId, 2.5, "kg", "weekend prep overshoot");
    REQUIRE_FALSE(wasteId.empty());
    REQUIRE(eventReceived);
    REQUIRE(eventReason == "weekend prep overshoot");

    auto records = f.service.listWaste();
    REQUIRE(records.size() == 1);
    REQUIRE(records[0].quantity() == 2.5);
    REQUIRE(records[0].unit() == "kg");
}

TEST_CASE("FinanceService calculate margin") {
    FinanceTestFixture f;

    auto id = f.service.addCostItem("Salmon", 850, 900, 2800);

    auto margin = f.service.calculateMargin(id);
    REQUIRE(margin.has_value());
    REQUIRE_THAT(*margin, Catch::Matchers::WithinAbs(69.6, 0.1));

    // Non-existent item returns nullopt
    auto noMargin = f.service.calculateMargin("nonexistent");
    REQUIRE_FALSE(noMargin.has_value());
}

TEST_CASE("FinanceService food cost report") {
    FinanceTestFixture f;

    f.service.addCostItem("Salmon", 850, 900, 2800);
    f.service.addCostItem("Risotto", 400, 450, 1800);

    auto ratio = f.service.foodCostReport();
    // Total ingredient cost: 850 + 400 = 1250
    // Total revenue: 2800 + 1800 = 4600
    // Ratio: 1250 / 4600 * 100 = 27.17...%
    REQUIRE_THAT(ratio.percent(),
                 Catch::Matchers::WithinAbs(27.2, 0.1));
}

TEST_CASE("FinanceService update cost item") {
    FinanceTestFixture f;

    auto id = f.service.addCostItem("Salmon", 850, 900, 2800);

    auto result = f.service.updateCostItem(id, 950, 1000);
    REQUIRE(result.isOk());

    auto items = f.service.listCostItems();
    REQUIRE(items.size() == 1);
    REQUIRE(items[0].ingredientCost().cents == 950);
    REQUIRE(items[0].supplierPrice().cents == 1000);

    // Non-existent item returns error
    auto failResult = f.service.updateCostItem("nonexistent", 100, 200);
    REQUIRE(failResult.isError());
}
