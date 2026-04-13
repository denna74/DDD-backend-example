#include <catch2/catch_test_macros.hpp>
#include "kitchen/domain/dish.h"
#include "kitchen/domain/station.h"

TEST_CASE("Dish knows its prep time and station") {
    kitchen::Dish dish(
        shared::Id{"dish-1"}, "Salmon", 12, "pan-sear", kitchen::Station::Grill);

    REQUIRE(dish.id().value == "dish-1");
    REQUIRE(dish.name() == "Salmon");
    REQUIRE(dish.prepTimeMinutes() == 12);
    REQUIRE(dish.cookingMethod() == "pan-sear");
    REQUIRE(dish.station() == kitchen::Station::Grill);
    REQUIRE(dish.isAvailable());
}

TEST_CASE("Mark dish out of stock") {
    kitchen::Dish dish(
        shared::Id{"dish-2"}, "Risotto", 22, "simmer", kitchen::Station::Sauce);

    REQUIRE(dish.isAvailable());
    dish.markOutOfStock();
    REQUIRE_FALSE(dish.isAvailable());
}

TEST_CASE("Restore dish availability") {
    kitchen::Dish dish(
        shared::Id{"dish-3"}, "Caesar Salad", 5, "raw", kitchen::Station::Cold, false);

    REQUIRE_FALSE(dish.isAvailable());
    dish.restore();
    REQUIRE(dish.isAvailable());
}

TEST_CASE("Station converts to and from string") {
    REQUIRE(kitchen::stationToString(kitchen::Station::Grill) == "grill");
    REQUIRE(kitchen::stationToString(kitchen::Station::Sauce) == "sauce");
    REQUIRE(kitchen::stationToString(kitchen::Station::Cold) == "cold");
    REQUIRE(kitchen::stationToString(kitchen::Station::Pastry) == "pastry");

    REQUIRE(kitchen::stationFromString("grill") == kitchen::Station::Grill);
    REQUIRE(kitchen::stationFromString("sauce") == kitchen::Station::Sauce);
    REQUIRE(kitchen::stationFromString("cold") == kitchen::Station::Cold);
    REQUIRE(kitchen::stationFromString("pastry") == kitchen::Station::Pastry);

    REQUIRE_THROWS_AS(kitchen::stationFromString("unknown"), std::invalid_argument);
}
