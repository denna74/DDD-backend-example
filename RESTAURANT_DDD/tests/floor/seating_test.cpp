#include <catch2/catch_test_macros.hpp>
#include "floor/domain/seating.h"

TEST_CASE("Walk-in seating tracks covers") {
    auto seating = floor_ctx::Seating::walkIn(
        shared::Id{"s-1"}, shared::Id{"t-1"}, 3);

    REQUIRE(seating.id().value == "s-1");
    REQUIRE(seating.tableId().value == "t-1");
    REQUIRE(seating.coverCount() == 3);
    REQUIRE(seating.isWalkIn());
    REQUIRE(seating.reservationName().empty());
    REQUIRE_FALSE(seating.seatedAt().empty());
    REQUIRE(seating.clearedAt().empty());
}

TEST_CASE("Reservation seating tracks name") {
    auto seating = floor_ctx::Seating::reservation(
        shared::Id{"s-2"}, shared::Id{"t-2"}, 4, "Smith");

    REQUIRE(seating.coverCount() == 4);
    REQUIRE_FALSE(seating.isWalkIn());
    REQUIRE(seating.reservationName() == "Smith");
    REQUIRE_FALSE(seating.seatedAt().empty());
    REQUIRE(seating.clearedAt().empty());
}

TEST_CASE("Clear seating records time") {
    auto seating = floor_ctx::Seating::walkIn(
        shared::Id{"s-3"}, shared::Id{"t-3"}, 2);

    REQUIRE(seating.clearedAt().empty());
    seating.clear();
    REQUIRE_FALSE(seating.clearedAt().empty());
}
