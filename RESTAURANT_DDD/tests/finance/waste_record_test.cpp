#include <catch2/catch_test_macros.hpp>
#include "finance/domain/waste_record.h"

TEST_CASE("WasteRecord captures what was wasted and why") {
    finance::WasteRecord record(
        shared::Id{"wr-1"}, shared::Id{"ci-1"},
        2.5, "kg", "weekend prep overshoot");

    REQUIRE(record.id().value == "wr-1");
    REQUIRE(record.costItemId().value == "ci-1");
    REQUIRE(record.quantity() == 2.5);
    REQUIRE(record.unit() == "kg");
    REQUIRE(record.reason() == "weekend prep overshoot");
    REQUIRE_FALSE(record.recordedAt().empty());
}
