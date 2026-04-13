#include "shared_kernel/db.h"
#include "shared_kernel/event_bus.h"

#include "kitchen/ports/i_kitchen_event_publisher.h"
#include "kitchen/adapters/sqlite_kitchen_repository.h"
#include "kitchen/application/kitchen_service.h"
#include "kitchen/adapters/http_kitchen_controller.h"

#include "floor/ports/i_floor_event_publisher.h"
#include "floor/adapters/sqlite_floor_repository.h"
#include "floor/application/floor_service.h"
#include "floor/adapters/http_floor_controller.h"

#include "finance/ports/i_finance_event_publisher.h"
#include "finance/adapters/sqlite_finance_repository.h"
#include "finance/application/finance_service.h"
#include "finance/adapters/http_finance_controller.h"

#include <httplib.h>
#include <iostream>
#include <string>
#include <cstring>

// ---------------------------------------------------------------------------
// EventBus publisher adapters — implement each context's IXxxEventPublisher
// by delegating to the shared EventBus.
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Argument parsing
// ---------------------------------------------------------------------------

static void parseArgs(int argc, char* argv[], std::string& dbPath, int& port) {
    dbPath = "restaurant.db";
    port   = 8082;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--db") == 0 && i + 1 < argc) {
            dbPath = argv[++i];
        } else if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    std::string dbPath;
    int port;
    parseArgs(argc, argv, dbPath, port);

    // 1. Database
    Database db(dbPath);
    db.execFile("db/schema.sql");

    // 2. Shared event bus
    shared::EventBus eventBus;

    // 3. EventBus publisher adapters
    KitchenEventBusPublisher kitchenPublisher(eventBus);
    FloorEventBusPublisher   floorPublisher(eventBus);
    FinanceEventBusPublisher financePublisher(eventBus);

    // 4. Repositories
    kitchen::SqliteKitchenRepository kitchenRepo(db);
    floor_ctx::SqliteFloorRepository floorRepo(db);
    finance::SqliteFinanceRepository financeRepo(db);

    // 5. Application services
    kitchen::KitchenService   kitchenService(kitchenRepo, kitchenPublisher);
    floor_ctx::FloorService   floorService(floorRepo, floorPublisher);
    finance::FinanceService   financeService(financeRepo, financePublisher);

    // 6. Cross-context event subscriptions
    //    Kitchen DishMarkedOutOfStock -> Floor markSoldOutByName
    eventBus.subscribe<kitchen::DishMarkedOutOfStock>(
        [&floorService](const kitchen::DishMarkedOutOfStock& e) {
            floorService.markSoldOutByName(e.dishName);
        });

    // 7. HTTP server + controllers
    httplib::Server server;

    kitchen::HttpKitchenController   kitchenController(kitchenService);
    floor_ctx::HttpFloorController   floorController(floorService);
    finance::HttpFinanceController   financeController(financeService);

    kitchenController.registerRoutes(server);
    floorController.registerRoutes(server);
    financeController.registerRoutes(server);

    // 8. Start
    std::cout << "Restaurant DDD server starting on port " << port << std::endl;
    std::cout << "  Database: " << dbPath << std::endl;
    std::cout << "  Endpoints:" << std::endl;
    std::cout << "    Kitchen  -> /api/kitchen/*" << std::endl;
    std::cout << "    Floor    -> /api/floor/*" << std::endl;
    std::cout << "    Finance  -> /api/finance/*" << std::endl;

    server.listen("0.0.0.0", port);
    return 0;
}
