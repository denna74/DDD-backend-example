#include "services/reservation_service.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

static std::string generateHexId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << dist(gen)
        << std::setw(16) << dist(gen);
    return oss.str();
}

static std::string nowIso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc{};
#ifdef _WIN32
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    return buf;
}

ReservationService::ReservationService(ReservationRepository& repo) : repo_(repo) {}

std::string ReservationService::generateId() { return generateHexId(); }
std::string ReservationService::currentTimestamp() { return nowIso8601(); }

std::string ReservationService::createReservation(
    const std::string& table_id,
    int guest_count,
    const std::string& type,
    const std::string& name)
{
    Reservation r;
    r.id               = generateId();
    r.table_id         = table_id;
    r.guest_count      = guest_count;
    r.type             = type;
    r.reservation_name = name;
    r.seated_at        = currentTimestamp();
    repo_.create(r);
    return r.id;
}

std::vector<Reservation> ReservationService::getReservations() {
    return repo_.findAll();
}

std::optional<Reservation> ReservationService::getReservation(const std::string& id) {
    return repo_.findById(id);
}

void ReservationService::updateReservation(const std::string& id, const std::string& cleared_at) {
    repo_.update(id, cleared_at);
}
