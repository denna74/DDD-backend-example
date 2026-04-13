#pragma once
#include <string>

struct Item {
    std::string id;
    std::string name;
    std::string description;
    int price_cents = 0;
    int prep_time_minutes = 0;
    std::string cooking_method;
    std::string station;
    int ingredient_cost_cents = 0;
    int supplier_price_cents = 0;
    bool is_available = true;
};
