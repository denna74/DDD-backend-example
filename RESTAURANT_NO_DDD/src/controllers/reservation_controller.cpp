#include "controllers/reservation_controller.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static json reservationToJson(const Reservation& r) {
    return json{
        {"id",               r.id},
        {"table_id",         r.table_id},
        {"guest_count",      r.guest_count},
        {"type",             r.type},
        {"reservation_name", r.reservation_name},
        {"seated_at",        r.seated_at},
        {"cleared_at",       r.cleared_at}
    };
}

ReservationController::ReservationController(ReservationService& svc) : svc_(svc) {}

void ReservationController::registerRoutes(httplib::Server& server) {
    // GET /api/reservations
    server.Get("/api/reservations", [this](const httplib::Request&, httplib::Response& res) {
        auto reservations = svc_.getReservations();
        json arr = json::array();
        for (const auto& r : reservations) {
            arr.push_back(reservationToJson(r));
        }
        res.set_content(arr.dump(), "application/json");
    });

    // POST /api/reservations
    server.Post("/api/reservations", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string table_id         = body.value("table_id", "");
        int guest_count              = body.value("guest_count", 0);
        std::string type             = body.value("type", "RESERVED");
        std::string reservation_name = body.value("reservation_name", "");

        std::string id = svc_.createReservation(table_id, guest_count, type, reservation_name);
        res.status = 201;
        res.set_content(json{{"id", id}}.dump(), "application/json");
    });

    // PUT /api/reservations/:id
    server.Put(R"(/api/reservations/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1].str();
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
            return;
        }
        std::string cleared_at = body.value("cleared_at", "");
        svc_.updateReservation(id, cleared_at);
        res.set_content(R"({"ok":true})", "application/json");
    });
}
