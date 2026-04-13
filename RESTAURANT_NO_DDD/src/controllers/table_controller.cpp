#include "controllers/table_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static json tableToJson(const Table& table) {
    return json{
        {"id",           table.id},
        {"table_number", table.table_number},
        {"capacity",     table.capacity},
        {"status",       table.status}
    };
}

TableController::TableController(TableService& svc) : svc_(svc) {}

void TableController::registerRoutes(httplib::Server& server) {
    // GET /api/tables
    server.Get("/api/tables", [this](const httplib::Request&, httplib::Response& res) {
        auto tables = svc_.getTables();
        json arr = json::array();
        for (const auto& table : tables) {
            arr.push_back(tableToJson(table));
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/tables
    server.Post("/api/tables", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        int table_number = body.value("table_number", 0);
        int capacity     = body.value("capacity", 0);
        std::string id = svc_.createTable(table_number, capacity);
        res.status = 201;
        res.set_content(json{{"id", id}}.dump(), "application/json");
    });

    // PUT /api/tables/:id/status
    server.Put(R"(/api/tables/([^/]+)/status)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1].str();
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string status = body.value("status", "");
        svc_.updateTableStatus(id, status);
        res.set_content(R"({"ok":true})", "application/json");
    });
}
