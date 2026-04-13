#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "finance/domain/cost_item.h"

TEST_CASE("CostItem knows costs and margin") {
    finance::CostItem item(
        shared::Id{"ci-1"}, "Salmon",
        shared::Money(850), shared::Money(900), shared::Money(2800));

    REQUIRE(item.id().value == "ci-1");
    REQUIRE(item.name() == "Salmon");
    REQUIRE(item.ingredientCost().cents == 850);
    REQUIRE(item.supplierPrice().cents == 900);
    REQUIRE(item.sellingPrice().cents == 2800);
}

TEST_CASE("CostItem calculates margin") {
    // Salmon: (2800 - 850) / 2800 * 100 = 69.642...%
    finance::CostItem item(
        shared::Id{"ci-2"}, "Salmon",
        shared::Money(850), shared::Money(900), shared::Money(2800));

    REQUIRE_THAT(item.marginPercent(),
                 Catch::Matchers::WithinAbs(69.6, 0.1));
}

TEST_CASE("CostItem update supplier price") {
    finance::CostItem item(
        shared::Id{"ci-3"}, "Salmon",
        shared::Money(850), shared::Money(900), shared::Money(2800));

    item.updateSupplierPrice(shared::Money(950));
    REQUIRE(item.supplierPrice().cents == 950);
}
