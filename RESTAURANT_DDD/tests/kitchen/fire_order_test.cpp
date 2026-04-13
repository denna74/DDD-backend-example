#include <catch2/catch_test_macros.hpp>
#include "kitchen/domain/fire_order.h"

TEST_CASE("Create fire order for a table with timing coordination") {
    kitchen::FireOrder order(
        shared::Id{"fo-1"}, 5, shared::Timestamp::now());

    REQUIRE(order.id().value == "fo-1");
    REQUIRE(order.tableNumber() == 5);
    REQUIRE(order.status() == kitchen::FireOrderStatus::Coordinating);
    REQUIRE(order.lines().empty());
}

TEST_CASE("Add lines with staggered fire offsets") {
    kitchen::FireOrder order(
        shared::Id{"fo-2"}, 3, shared::Timestamp::now());

    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-salmon"}, 0);
    order.addLine(shared::Id{"line-2"}, shared::Id{"dish-risotto"}, 5);
    order.addLine(shared::Id{"line-3"}, shared::Id{"dish-fondant"}, 10);

    REQUIRE(order.lines().size() == 3);
    REQUIRE(order.lines()[0].fireAtOffsetMinutes() == 0);
    REQUIRE(order.lines()[1].fireAtOffsetMinutes() == 5);
    REQUIRE(order.lines()[2].fireAtOffsetMinutes() == 10);
}

TEST_CASE("Fire dish updates line status and records timing") {
    kitchen::FireOrder order(
        shared::Id{"fo-3"}, 7, shared::Timestamp::now());
    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-1"}, 0);

    auto result = order.fireDish(shared::Id{"line-1"});
    REQUIRE(result.isOk());
    REQUIRE(order.lines()[0].status() == kitchen::FireLineStatus::Fired);
    REQUIRE_FALSE(order.lines()[0].firedAt().empty());
    REQUIRE(order.status() == kitchen::FireOrderStatus::InProgress);
}

TEST_CASE("Cannot fire a dish that is already fired") {
    kitchen::FireOrder order(
        shared::Id{"fo-4"}, 2, shared::Timestamp::now());
    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-1"}, 0);

    order.fireDish(shared::Id{"line-1"});
    auto result = order.fireDish(shared::Id{"line-1"});
    REQUIRE(result.isError());
    REQUIRE(result.error == "Dish already fired");
}

TEST_CASE("Plate dish updates line status") {
    kitchen::FireOrder order(
        shared::Id{"fo-5"}, 4, shared::Timestamp::now());
    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-1"}, 0);

    order.fireDish(shared::Id{"line-1"});
    auto result = order.plateDish(shared::Id{"line-1"});
    REQUIRE(result.isOk());
    REQUIRE(order.lines()[0].status() == kitchen::FireLineStatus::Plated);
    REQUIRE_FALSE(order.lines()[0].platedAt().empty());
}

TEST_CASE("Cannot plate a dish that has not been fired") {
    kitchen::FireOrder order(
        shared::Id{"fo-6"}, 1, shared::Timestamp::now());
    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-1"}, 0);

    auto result = order.plateDish(shared::Id{"line-1"});
    REQUIRE(result.isError());
    REQUIRE(result.error == "Dish has not been fired yet");
}

TEST_CASE("All dishes plated marks fire order as plated") {
    kitchen::FireOrder order(
        shared::Id{"fo-7"}, 9, shared::Timestamp::now());
    order.addLine(shared::Id{"line-1"}, shared::Id{"dish-1"}, 0);
    order.addLine(shared::Id{"line-2"}, shared::Id{"dish-2"}, 5);

    order.fireDish(shared::Id{"line-1"});
    order.fireDish(shared::Id{"line-2"});
    order.plateDish(shared::Id{"line-1"});

    REQUIRE(order.status() == kitchen::FireOrderStatus::InProgress);

    order.plateDish(shared::Id{"line-2"});
    REQUIRE(order.status() == kitchen::FireOrderStatus::Plated);
    REQUIRE(order.allPlated());
}
