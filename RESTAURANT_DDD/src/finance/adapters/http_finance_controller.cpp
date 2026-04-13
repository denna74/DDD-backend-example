#include "finance/adapters/http_finance_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace finance {

HttpFinanceController::HttpFinanceController(FinanceService& service)
    : service_(service) {}

void HttpFinanceController::registerRoutes(httplib::Server& server) {

    // GET /api/finance/cost-items
    server.Get("/api/finance/cost-items", [this](const httplib::Request&, httplib::Response& res) {
        auto items = service_.listCostItems();
        json arr = json::array();
        for (const auto& item : items) {
            arr.push_back({
                {"id", item.id().value},
                {"name", item.name()},
                {"ingredient_cost_cents", item.ingredientCost().cents},
                {"supplier_price_cents", item.supplierPrice().cents},
                {"selling_price_cents", item.sellingPrice().cents},
                {"margin_percent", item.marginPercent()}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/finance/cost-items
    server.Post("/api/finance/cost-items", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            auto id = service_.addCostItem(
                body.at("name").get<std::string>(),
                body.at("ingredient_cost_cents").get<int>(),
                body.at("supplier_price_cents").get<int>(),
                body.at("selling_price_cents").get<int>()
            );
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // PUT /api/finance/cost-items/:id
    server.Put("/api/finance/cost-items/:id", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto id = req.path_params.at("id");
            auto body = json::parse(req.body);
            auto result = service_.updateCostItem(
                id,
                body.at("ingredient_cost_cents").get<int>(),
                body.at("supplier_price_cents").get<int>()
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

    // GET /api/finance/cost-items/:id/margin
    server.Get("/api/finance/cost-items/:id/margin", [this](const httplib::Request& req, httplib::Response& res) {
        auto id = req.path_params.at("id");
        auto margin = service_.calculateMargin(id);
        if (!margin.has_value()) {
            res.status = 404;
            res.set_content(json({{"error", "Cost item not found"}}).dump(), "application/json");
            return;
        }
        res.set_content(json({{"margin_percent", *margin}}).dump(), "application/json");
    });

    // POST /api/finance/waste
    server.Post("/api/finance/waste", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            auto id = service_.recordWaste(
                body.at("cost_item_id").get<std::string>(),
                body.at("quantity").get<double>(),
                body.at("unit").get<std::string>(),
                body.at("reason").get<std::string>()
            );
            res.status = 201;
            res.set_content(json({{"id", id}}).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 422;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // GET /api/finance/waste
    server.Get("/api/finance/waste", [this](const httplib::Request&, httplib::Response& res) {
        auto records = service_.listWaste();
        json arr = json::array();
        for (const auto& r : records) {
            arr.push_back({
                {"id", r.id().value},
                {"cost_item_id", r.costItemId().value},
                {"quantity", r.quantity()},
                {"unit", r.unit()},
                {"reason", r.reason()},
                {"recorded_at", r.recordedAt()}
            });
        }
        res.set_content(arr.dump(), "application/json");
    });

    // GET /api/finance/reports/food-cost-ratio
    server.Get("/api/finance/reports/food-cost-ratio", [this](const httplib::Request&, httplib::Response& res) {
        auto ratio = service_.foodCostReport();
        res.set_content(json({{"food_cost_ratio_percent", ratio.percent()}}).dump(), "application/json");
    });
}

} // namespace finance
