#pragma once
#include <string>

struct Reservation {
    std::string id;
    std::string table_id;
    int guest_count = 0;       // Sara calls these "covers"
    std::string type;          // "RESERVED" or "UNPLANNED" — Sara calls the latter "walk-in"
    std::string reservation_name;
    std::string seated_at;
    std::string cleared_at;
};
