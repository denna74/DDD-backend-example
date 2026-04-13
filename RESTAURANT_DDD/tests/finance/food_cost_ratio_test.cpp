#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "finance/domain/food_cost_ratio.h"

TEST_CASE("FoodCostRatio calculates percentage") {
    // 850 / 2800 * 100 = 30.357...%
    finance::FoodCostRatio ratio(shared::Money(850), shared::Money(2800));

    REQUIRE_THAT(ratio.percent(),
                 Catch::Matchers::WithinAbs(30.4, 0.1));
}
