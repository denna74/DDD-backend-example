#pragma once
#include "repositories/reservation_repository.h"
#include <string>
#include <vector>
#include <optional>

class ReservationService {
public:
    explicit ReservationService(ReservationRepository& repo);

    std::string createReservation(
        const std::string& table_id,
        int guest_count,
        const std::string& type,
        const std::string& name
    );
    std::vector<Reservation> getReservations();
    std::optional<Reservation> getReservation(const std::string& id);
    void updateReservation(const std::string& id, const std::string& cleared_at);

private:
    ReservationRepository& repo_;
    static std::string generateId();
    static std::string currentTimestamp();
};
