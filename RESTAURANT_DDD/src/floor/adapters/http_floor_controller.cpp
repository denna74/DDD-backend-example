#include "floor/adapters/http_floor_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace floor_ctx {

HttpFloorController::HttpFloorController(FloorService& service)
    : service_(service) {}

void HttpFloorController::registerRoutes(httplib::Server& server) {

    // GET /api/floor/tables
    server.Get("/api/floor/tables", [this](const httplib::Request&, httplib::Response& res) {
        auto tables = service_.listTables();
        json arr = json::array();
        for (const auto& t : tables) {
            arr.push_back({
                {"id", t.id().value},
                {"tableNumber", t.tableNumber()},
                {"capacity", t.capacity()},
                {"status", tableStatusToString(t.status())}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/floor/tables
    server.Post("/api/floor/tables", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            auto id = service_.addTable(
                body.at("table_number").get<int>(),
                body.at("capacity").get<int>()
            );
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /api/floor/tables/:id/seat-walk-in
    server.Post("/api/floor/tables/:id/seat-walk-in",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto tableId = req.path_params.at("id");
                auto body = json::parse(req.body);
                int partySize = body.at("party_size").get<int>();
                auto result = service_.seatWalkIn(tableId, partySize);
                if (result.isError()) {
                    // Distinguish not found vs business rule violation
                    if (result.error == "Table not found") {
                        res.status = 404;
                    } else {
                        res.status = 422;
                    }
                    res.set_content(json({{"error", result.error}}).dump(), "application/json");
                    return;
                }
                res.set_content(json({{"status", "seated"}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 422;
                res.set_content(json({{"error", e.what()}}).dump(), "application/json");
            }
        });

    // POST /api/floor/tables/:id/seat-reservation
    server.Post("/api/floor/tables/:id/seat-reservation",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto tableId = req.path_params.at("id");
                auto body = json::parse(req.body);
                int partySize = body.at("party_size").get<int>();
                auto name = body.at("reservation_name").get<std::string>();
                auto result = service_.seatReservation(tableId, partySize, name);
                if (result.isError()) {
                    if (result.error == "Table not found") {
                        res.status = 404;
                    } else {
                        res.status = 422;
                    }
                    res.set_content(json({{"error", result.error}}).dump(), "application/json");
                    return;
                }
                res.set_content(json({{"status", "seated"}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 422;
                res.set_content(json({{"error", e.what()}}).dump(), "application/json");
            }
        });

    // POST /api/floor/tables/:id/turn
    server.Post("/api/floor/tables/:id/turn",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto tableId = req.path_params.at("id");
            auto result = service_.turnTable(tableId);
            if (result.isError()) {
                if (result.error == "Table not found") {
                    res.status = 404;
                } else {
                    res.status = 422;
                }
                res.set_content(json({{"error", result.error}}).dump(), "application/json");
                return;
            }
            res.set_content(json({{"status", "turned"}}).dump(), "application/json");
        });

    // GET /api/floor/menu-items
    server.Get("/api/floor/menu-items", [this](const httplib::Request&, httplib::Response& res) {
        auto items = service_.listMenuItems();
        json arr = json::array();
        for (const auto& item : items) {
            arr.push_back({
                {"id", item.id().value},
                {"name", item.name()},
                {"description", item.description()},
                {"priceCents", item.price().cents},
                {"isAvailable", item.isAvailable()}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/floor/menu-items
    server.Post("/api/floor/menu-items", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            auto id = service_.addMenuItem(
                body.at("name").get<std::string>(),
                body.at("description").get<std::string>(),
                body.at("price_cents").get<int>()
            );
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // PUT /api/floor/menu-items/:id
    server.Put("/api/floor/menu-items/:id",
        [this](const httplib::Request& req, httplib::Response& res) {
            try {
                auto id = req.path_params.at("id");
                auto body = json::parse(req.body);
                auto result = service_.updateMenuItem(
                    id,
                    body.at("name").get<std::string>(),
                    body.at("description").get<std::string>(),
                    body.at("price_cents").get<int>()
                );
                if (result.isError()) {
                    res.status = 404;
                    res.set_content(json({{"error", result.error}}).dump(), "application/json");
                    return;
                }
                res.set_content(json({{"status", "updated"}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 422;
                res.set_content(json({{"error", e.what()}}).dump(), "application/json");
            }
        });

    // POST /api/floor/menu-items/:id/sold-out
    server.Post("/api/floor/menu-items/:id/sold-out",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto id = req.path_params.at("id");
            auto result = service_.markSoldOut(id);
            if (result.isError()) {
                res.status = 404;
                res.set_content(json({{"error", result.error}}).dump(), "application/json");
                return;
            }
            res.set_content(json({{"status", "sold_out"}}).dump(), "application/json");
        });

    // GET /api/floor/covers/tonight
    server.Get("/api/floor/covers/tonight", [this](const httplib::Request&, httplib::Response& res) {
        int covers = service_.countCoversTonight();
        res.set_content(json({{"covers", covers}}).dump(), "application/json");
    });
}

} // namespace floor_ctx
