#include "controllers/item_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static json itemToJson(const Item& item) {
    return json{
        {"id",                    item.id},
        {"name",                  item.name},
        {"description",           item.description},
        {"price_cents",           item.price_cents},
        {"prep_time_minutes",     item.prep_time_minutes},
        {"cooking_method",        item.cooking_method},
        {"station",               item.station},
        {"ingredient_cost_cents", item.ingredient_cost_cents},
        {"supplier_price_cents",  item.supplier_price_cents},
        {"is_available",          item.is_available}
    };
}

ItemController::ItemController(ItemService& svc) : svc_(svc) {}

void ItemController::registerRoutes(httplib::Server& server) {
    // GET /api/items
    server.Get("/api/items", [this](const httplib::Request&, httplib::Response& res) {
        auto items = svc_.getItems();
        json arr = json::array();
        for (const auto& item : items) {
            arr.push_back(itemToJson(item));
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/items
    server.Post("/api/items", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string name             = body.value("name", "");
        std::string description      = body.value("description", "");
        int price_cents              = body.value("price_cents", 0);
        int prep_time_minutes        = body.value("prep_time_minutes", 0);
        std::string cooking_method   = body.value("cooking_method", "");
        std::string station          = body.value("station", "");
        int ingredient_cost_cents    = body.value("ingredient_cost_cents", 0);
        int supplier_price_cents     = body.value("supplier_price_cents", 0);

        std::string id = svc_.createItem(name, description, price_cents, prep_time_minutes,
                                         cooking_method, station, ingredient_cost_cents,
                                         supplier_price_cents);
        res.status = 201;
        res.set_content(json{{"id", id}}.dump(), "application/json");
    });

    // PUT /api/items/:id
    server.Put(R"(/api/items/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1].str();
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string name             = body.value("name", "");
        std::string description      = body.value("description", "");
        int price_cents              = body.value("price_cents", 0);
        int prep_time_minutes        = body.value("prep_time_minutes", 0);
        std::string cooking_method   = body.value("cooking_method", "");
        std::string station          = body.value("station", "");
        int ingredient_cost_cents    = body.value("ingredient_cost_cents", 0);
        int supplier_price_cents     = body.value("supplier_price_cents", 0);

        svc_.updateItem(id, name, description, price_cents, prep_time_minutes,
                        cooking_method, station, ingredient_cost_cents, supplier_price_cents);
        res.set_content(R"({"ok":true})", "application/json");
    });

    // PATCH /api/items/:id/availability
    server.Patch(R"(/api/items/([^/]+)/availability)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1].str();
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        bool is_available = body.value("is_available", true);
        svc_.updateAvailability(id, is_available);
        res.set_content(R"({"ok":true})", "application/json");
    });
}
