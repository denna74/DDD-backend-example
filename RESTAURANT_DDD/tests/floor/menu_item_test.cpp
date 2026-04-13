#include <catch2/catch_test_macros.hpp>
#include "floor/domain/menu_item.h"

TEST_CASE("MenuItem knows price and description") {
    floor_ctx::MenuItem item(
        shared::Id{"mi-1"}, "Salmon", "Pan-seared Atlantic salmon",
        shared::Money(2800));

    REQUIRE(item.id().value == "mi-1");
    REQUIRE(item.name() == "Salmon");
    REQUIRE(item.description() == "Pan-seared Atlantic salmon");
    REQUIRE(item.price().cents == 2800);
    REQUIRE(item.isAvailable());
}

TEST_CASE("Mark menu item sold out") {
    floor_ctx::MenuItem item(
        shared::Id{"mi-2"}, "Risotto", "Truffle risotto",
        shared::Money(2200));

    REQUIRE(item.isAvailable());
    item.markSoldOut();
    REQUIRE_FALSE(item.isAvailable());

    // Restore it
    item.restore();
    REQUIRE(item.isAvailable());
}
