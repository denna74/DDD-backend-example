#pragma once
#include <string>
#include <optional>

struct OrderLine {
    std::string id;
    std::string order_id;
    std::string item_id;
    std::string status;
    std::optional<int> fire_at_offset_minutes;
    std::string fired_at;
    std::string plated_at;
};
