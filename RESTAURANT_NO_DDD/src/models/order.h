#pragma once
#include <string>

struct Order {
    std::string id;
    int table_number = 0;
    std::string status;
    std::string created_at;
};
