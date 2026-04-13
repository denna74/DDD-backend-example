#include "db.h"
#include "repositories/item_repository.h"
#include "repositories/order_repository.h"
#include "repositories/table_repository.h"
#include "repositories/reservation_repository.h"
#include "services/item_service.h"
#include "services/order_service.h"
#include "services/table_service.h"
#include "services/reservation_service.h"
#include "controllers/item_controller.h"
#include "controllers/order_controller.h"
#include "controllers/table_controller.h"
#include "controllers/reservation_controller.h"
#include <httplib.h>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string dbPath = "restaurant.db";
    int port = 8081;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--db" && i + 1 < argc) dbPath = argv[++i];
        if (arg == "--port" && i + 1 < argc) port = std::stoi(argv[++i]);
    }

    Database db(dbPath);
    db.execFile("db/schema.sql");

    // Wire repos -> services -> controllers
    ItemRepository itemRepo(db);
    OrderRepository orderRepo(db);
    TableRepository tableRepo(db);
    ReservationRepository resRepo(db);

    ItemService itemSvc(itemRepo);
    OrderService orderSvc(orderRepo);
    TableService tableSvc(tableRepo);
    ReservationService resSvc(resRepo);

    httplib::Server server;
    ItemController(itemSvc).registerRoutes(server);
    OrderController(orderSvc).registerRoutes(server);
    TableController(tableSvc).registerRoutes(server);
    ReservationController(resSvc).registerRoutes(server);

    std::cout << "Non-DDD Restaurant API running on port " << port << std::endl;
    server.listen("0.0.0.0", port);
    return 0;
}
