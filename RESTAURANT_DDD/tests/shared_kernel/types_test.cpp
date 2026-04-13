#include <catch2/catch_test_macros.hpp>
#include "shared_kernel/types.h"

TEST_CASE("Id generates unique values") {
    auto id1 = shared::Id::generate();
    auto id2 = shared::Id::generate();
    REQUIRE_FALSE(id1.value.empty());
    REQUIRE(id1 != id2);
}

TEST_CASE("Id equality") {
    shared::Id a{"abc123"};
    shared::Id b{"abc123"};
    shared::Id c{"def456"};
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("Money arithmetic") {
    shared::Money a(1000);
    shared::Money b(250);
    REQUIRE((a + b).cents == 1250);
    REQUIRE((a - b).cents == 750);
    REQUIRE(a.toDollars() == 10.0);
}

TEST_CASE("Money equality") {
    REQUIRE(shared::Money(500) == shared::Money(500));
    REQUIRE_FALSE(shared::Money(500) == shared::Money(600));
}

TEST_CASE("Timestamp now produces ISO 8601") {
    auto ts = shared::Timestamp::now();
    REQUIRE_FALSE(ts.value.empty());
    REQUIRE(ts.value.find('T') != std::string::npos);
    REQUIRE(ts.value.back() == 'Z');
}

TEST_CASE("Result ok holds value") {
    auto result = shared::Result<int>::ok(42);
    REQUIRE(result.isOk());
    REQUIRE(result.value.value() == 42);
}

TEST_CASE("Result fail holds error") {
    auto result = shared::Result<int>::fail("something went wrong");
    REQUIRE(result.isError());
    REQUIRE(result.error == "something went wrong");
}

TEST_CASE("Result<void> ok") {
    auto result = shared::Result<void>::ok();
    REQUIRE(result.isOk());
}

TEST_CASE("Result<void> fail") {
    auto result = shared::Result<void>::fail("oops");
    REQUIRE(result.isError());
    REQUIRE(result.error == "oops");
}
