#include "repositories/reservation_repository.h"
#include <stdexcept>

// Maps seatings table columns to Reservation model:
//   cover_count  -> guest_count
//   is_walk_in   -> type ("UNPLANNED" if 1, "RESERVED" if 0)

Reservation ReservationRepository::rowToReservation(sqlite3_stmt* stmt) {
    Reservation r;
    r.id               = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    r.table_id         = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    r.guest_count      = sqlite3_column_int(stmt, 2);   // cover_count column
    int is_walk_in     = sqlite3_column_int(stmt, 3);
    r.type             = is_walk_in ? "UNPLANNED" : "RESERVED";
    auto rname         = sqlite3_column_text(stmt, 4);
    r.reservation_name = rname ? reinterpret_cast<const char*>(rname) : "";
    auto seated        = sqlite3_column_text(stmt, 5);
    r.seated_at        = seated  ? reinterpret_cast<const char*>(seated)  : "";
    auto cleared       = sqlite3_column_text(stmt, 6);
    r.cleared_at       = cleared ? reinterpret_cast<const char*>(cleared) : "";
    return r;
}

void ReservationRepository::create(const Reservation& reservation) {
    const char* sql =
        "INSERT INTO seatings (id, table_id, cover_count, is_walk_in, reservation_name, "
        "seated_at, cleared_at) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ReservationRepository::create prepare failed");
    }
    sqlite3_bind_text(stmt, 1, reservation.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, reservation.table_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, reservation.guest_count);
    sqlite3_bind_int(stmt, 4, reservation.type == "UNPLANNED" ? 1 : 0);
    if (reservation.reservation_name.empty()) {
        sqlite3_bind_null(stmt, 5);
    } else {
        sqlite3_bind_text(stmt, 5, reservation.reservation_name.c_str(), -1, SQLITE_STATIC);
    }
    if (reservation.seated_at.empty()) {
        sqlite3_bind_null(stmt, 6);
    } else {
        sqlite3_bind_text(stmt, 6, reservation.seated_at.c_str(), -1, SQLITE_STATIC);
    }
    if (reservation.cleared_at.empty()) {
        sqlite3_bind_null(stmt, 7);
    } else {
        sqlite3_bind_text(stmt, 7, reservation.cleared_at.c_str(), -1, SQLITE_STATIC);
    }
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<Reservation> ReservationRepository::findById(const std::string& id) {
    const char* sql =
        "SELECT id, table_id, cover_count, is_walk_in, reservation_name, seated_at, cleared_at "
        "FROM seatings WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ReservationRepository::findById prepare failed");
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    std::optional<Reservation> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = rowToReservation(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Reservation> ReservationRepository::findAll() {
    const char* sql =
        "SELECT id, table_id, cover_count, is_walk_in, reservation_name, seated_at, cleared_at "
        "FROM seatings;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ReservationRepository::findAll prepare failed");
    }
    std::vector<Reservation> reservations;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        reservations.push_back(rowToReservation(stmt));
    }
    sqlite3_finalize(stmt);
    return reservations;
}

void ReservationRepository::update(const std::string& id, const std::string& cleared_at) {
    const char* sql = "UPDATE seatings SET cleared_at=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("ReservationRepository::update prepare failed");
    }
    sqlite3_bind_text(stmt, 1, cleared_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
