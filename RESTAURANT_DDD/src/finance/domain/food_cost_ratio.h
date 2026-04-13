#pragma once
#include "shared_kernel/types.h"

namespace finance {

struct FoodCostRatio {
    shared::Money ingredientCost;
    shared::Money revenue;

    FoodCostRatio(shared::Money cost, shared::Money rev)
        : ingredientCost(cost), revenue(rev) {}

    double percent() const {
        if (revenue.cents == 0) return 0.0;
        return static_cast<double>(ingredientCost.cents) / revenue.cents * 100.0;
    }
};

} // namespace finance
