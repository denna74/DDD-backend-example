#pragma once
#include "models/reservation.h"
#include "db.h"
#include <vector>
#include <optional>

class ReservationRepository {
public:
    explicit ReservationRepository(Database& db) : db_(db) {}
    void create(const Reservation& reservation);
    std::optional<Reservation> findById(const std::string& id);
    std::vector<Reservation> findAll();
    void update(const std::string& id, const std::string& cleared_at);
private:
    Database& db_;
    static Reservation rowToReservation(sqlite3_stmt* stmt);
};
