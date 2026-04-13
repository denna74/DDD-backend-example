#include "kitchen/adapters/http_kitchen_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace kitchen {

HttpKitchenController::HttpKitchenController(KitchenService& service)
    : service_(service) {}

void HttpKitchenController::registerRoutes(httplib::Server& server) {

    // GET /api/kitchen/dishes
    server.Get("/api/kitchen/dishes", [this](const httplib::Request&, httplib::Response& res) {
        auto dishes = service_.listDishes();
        json arr = json::array();
        for (const auto& d : dishes) {
            arr.push_back({
                {"id", d.id().value},
                {"name", d.name()},
                {"prepTimeMinutes", d.prepTimeMinutes()},
                {"cookingMethod", d.cookingMethod()},
                {"station", stationToString(d.station())},
                {"isAvailable", d.isAvailable()}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/kitchen/dishes
    server.Post("/api/kitchen/dishes", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            auto id = service_.addDish(
                body.at("name").get<std::string>(),
                body.at("prepTimeMinutes").get<int>(),
                body.at("cookingMethod").get<std::string>(),
                body.at("station").get<std::string>()
            );
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // POST /api/kitchen/dishes/:id/out-of-stock
    server.Post("/api/kitchen/dishes/:id/out-of-stock", [this](const httplib::Request& req, httplib::Response& res) {
        auto id = req.path_params.at("id");
        auto result = service_.markDishOutOfStock(id);
        if (result.isError()) {
            res.status = 404;
            res.set_content(json({{"error", result.error}}).dump(), "application/json");
            return;
        }
        res.set_content(json({{"status", "out_of_stock"}}).dump(), "application/json");
    });

    // POST /api/kitchen/dishes/:id/restore
    server.Post("/api/kitchen/dishes/:id/restore", [this](const httplib::Request& req, httplib::Response& res) {
        auto id = req.path_params.at("id");
        auto result = service_.restoreDish(id);
        if (result.isError()) {
            res.status = 404;
            res.set_content(json({{"error", result.error}}).dump(), "application/json");
            return;
        }
        res.set_content(json({{"status", "available"}}).dump(), "application/json");
    });

    // POST /api/kitchen/fire-orders
    server.Post("/api/kitchen/fire-orders", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            CreateFireOrderRequest request;
            request.tableNumber = body.at("tableNumber").get<int>();
            for (const auto& d : body.at("dishes")) {
                request.dishes.push_back({
                    d.at("dishId").get<std::string>(),
                    d.at("fireAtOffsetMinutes").get<int>()
                });
            }
            auto id = service_.createFireOrder(request);
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /api/kitchen/fire-orders
    server.Get("/api/kitchen/fire-orders", [this](const httplib::Request&, httplib::Response& res) {
        auto orders = service_.listFireOrders();
        json arr = json::array();
        for (const auto& o : orders) {
            json lines = json::array();
            for (const auto& l : o.lines()) {
                lines.push_back({
                    {"id", l.id().value},
                    {"dishId", l.dishId().value},
                    {"fireAtOffsetMinutes", l.fireAtOffsetMinutes()},
                    {"status", fireLineStatusToString(l.status())},
                    {"firedAt", l.firedAt().value},
                    {"platedAt", l.platedAt().value}
                });
            }
            arr.push_back({
                {"id", o.id().value},
                {"tableNumber", o.tableNumber()},
                {"status", fireOrderStatusToString(o.status())},
                {"createdAt", o.createdAt().value},
                {"lines", lines}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // GET /api/kitchen/fire-orders/:id
    server.Get("/api/kitchen/fire-orders/:id", [this](const httplib::Request& req, httplib::Response& res) {
        auto id = req.path_params.at("id");
        auto order = service_.getFireOrder(id);
        if (!order) {
            res.status = 404;
            res.set_content(json({{"error", "Fire order not found"}}).dump(), "application/json");
            return;
        }
        json lines = json::array();
        for (const auto& l : order->lines()) {
            lines.push_back({
                {"id", l.id().value},
                {"dishId", l.dishId().value},
                {"fireAtOffsetMinutes", l.fireAtOffsetMinutes()},
                {"status", fireLineStatusToString(l.status())},
                {"firedAt", l.firedAt().value},
                {"platedAt", l.platedAt().value}
            });
        }
        json j = {
            {"id", order->id().value},
            {"tableNumber", order->tableNumber()},
            {"status", fireOrderStatusToString(order->status())},
            {"createdAt", order->createdAt().value},
            {"lines", lines}
        };
        res.set_content(j.dump(), "application/json");
    });

    // POST /api/kitchen/fire-orders/:id/lines/:lineId/fire
    server.Post("/api/kitchen/fire-orders/:id/lines/:lineId/fire",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto orderId = req.path_params.at("id");
            auto lineId = req.path_params.at("lineId");
            auto result = service_.fireDish(orderId, lineId);
            if (result.isError()) {
                res.status = 422;
                res.set_content(json({{"error", result.error}}).dump(), "application/json");
                return;
            }
            res.set_content(json({{"status", "fired"}}).dump(), "application/json");
        });

    // POST /api/kitchen/fire-orders/:id/lines/:lineId/plate
    server.Post("/api/kitchen/fire-orders/:id/lines/:lineId/plate",
        [this](const httplib::Request& req, httplib::Response& res) {
            auto orderId = req.path_params.at("id");
            auto lineId = req.path_params.at("lineId");
            auto result = service_.plateDish(orderId, lineId);
            if (result.isError()) {
                res.status = 422;
                res.set_content(json({{"error", result.error}}).dump(), "application/json");
                return;
            }
            res.set_content(json({{"status", "plated"}}).dump(), "application/json");
        });
}

} // namespace kitchen
