#include <catch2/catch_test_macros.hpp>
#include "floor/domain/table.h"

TEST_CASE("Table starts available") {
    floor_ctx::Table table(shared::Id{"t-1"}, 1, 4);

    REQUIRE(table.id().value == "t-1");
    REQUIRE(table.tableNumber() == 1);
    REQUIRE(table.capacity() == 4);
    REQUIRE(table.status() == floor_ctx::TableStatus::Available);
}

TEST_CASE("Occupy table changes status") {
    floor_ctx::Table table(shared::Id{"t-2"}, 2, 6);

    auto result = table.occupy();
    REQUIRE(result.isOk());
    REQUIRE(table.status() == floor_ctx::TableStatus::Occupied);
}

TEST_CASE("Cannot occupy occupied table") {
    floor_ctx::Table table(shared::Id{"t-3"}, 3, 4);

    table.occupy();
    auto result = table.occupy();
    REQUIRE(result.isError());
    REQUIRE(result.error == "Table is not available or reserved");
}

TEST_CASE("Turn table makes available") {
    floor_ctx::Table table(shared::Id{"t-4"}, 4, 2);

    table.occupy();
    REQUIRE(table.status() == floor_ctx::TableStatus::Occupied);

    table.turn();
    REQUIRE(table.status() == floor_ctx::TableStatus::Available);
}

TEST_CASE("Reserve table") {
    floor_ctx::Table table(shared::Id{"t-5"}, 5, 8);

    table.reserve();
    REQUIRE(table.status() == floor_ctx::TableStatus::Reserved);

    // Can occupy a reserved table
    auto result = table.occupy();
    REQUIRE(result.isOk());
    REQUIRE(table.status() == floor_ctx::TableStatus::Occupied);
}
