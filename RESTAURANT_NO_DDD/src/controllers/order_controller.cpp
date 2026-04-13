#include "controllers/order_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static json orderLineToJson(const OrderLine& line) {
    json j{
        {"id",       line.id},
        {"order_id", line.order_id},
        {"item_id",  line.item_id},
        {"status",   line.status},
        {"fired_at", line.fired_at},
        {"plated_at",line.plated_at}
    };
    if (line.fire_at_offset_minutes.has_value()) {
        j["fire_at_offset_minutes"] = line.fire_at_offset_minutes.value();
    } else {
        j["fire_at_offset_minutes"] = nullptr;
    }
    return j;
}

static json orderWithLinesToJson(const OrderWithLines& owl) {
    json lines_arr = json::array();
    for (const auto& line : owl.lines) {
        lines_arr.push_back(orderLineToJson(line));
    }
    return json{
        {"id",           owl.order.id},
        {"table_number", owl.order.table_number},
        {"status",       owl.order.status},
        {"created_at",   owl.order.created_at},
        {"lines",        lines_arr}
    };
}

OrderController::OrderController(OrderService& svc) : svc_(svc) {}

void OrderController::registerRoutes(httplib::Server& server) {
    // GET /api/orders
    server.Get("/api/orders", [this](const httplib::Request&, httplib::Response& res) {
        auto orders = svc_.getOrders();
        json arr = json::array();
        for (const auto& owl : orders) {
            arr.push_back(orderWithLinesToJson(owl));
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/orders
    server.Post("/api/orders", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        int table_number = body.value("table_number", 0);
        std::vector<std::string> item_ids;
        if (body.contains("item_ids") && body["item_ids"].is_array()) {
            for (const auto& id_val : body["item_ids"]) {
                item_ids.push_back(id_val.get<std::string>());
            }
        }
        std::string id = svc_.createOrder(table_number, item_ids);
        res.status = 201;
        res.set_content(json{{"id", id}}.dump(), "application/json");
    });

    // PUT /api/orders/:id/status
    server.Put(R"(/api/orders/([^/]+)/status)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1].str();
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string status = body.value("status", "");
        svc_.updateOrderStatus(id, status);
        res.set_content(R"({"ok":true})", "application/json");
    });
}
