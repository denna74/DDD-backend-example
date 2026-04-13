#include <catch2/catch_test_macros.hpp>
#include "shared_kernel/event_bus.h"
#include "shared_kernel/domain_event.h"

struct TestEvent : shared::DomainEvent {
    std::string data;
    TestEvent(std::string d) : DomainEvent("TestEvent"), data(std::move(d)) {}
};

struct OtherEvent : shared::DomainEvent {
    int number;
    OtherEvent(int n) : DomainEvent("OtherEvent"), number(n) {}
};

TEST_CASE("EventBus delivers event to subscriber") {
    shared::EventBus bus;
    std::string received;
    bus.subscribe<TestEvent>([&](const TestEvent& e) { received = e.data; });
    bus.publish(TestEvent("hello"));
    REQUIRE(received == "hello");
}

TEST_CASE("EventBus delivers to multiple subscribers") {
    shared::EventBus bus;
    int count = 0;
    bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });
    bus.subscribe<TestEvent>([&](const TestEvent&) { count++; });
    bus.publish(TestEvent("x"));
    REQUIRE(count == 2);
}

TEST_CASE("EventBus does not cross-deliver between event types") {
    shared::EventBus bus;
    bool testCalled = false;
    bool otherCalled = false;
    bus.subscribe<TestEvent>([&](const TestEvent&) { testCalled = true; });
    bus.subscribe<OtherEvent>([&](const OtherEvent&) { otherCalled = true; });
    bus.publish(TestEvent("x"));
    REQUIRE(testCalled);
    REQUIRE_FALSE(otherCalled);
}

TEST_CASE("DomainEvent has id and timestamp") {
    TestEvent e("data");
    REQUIRE_FALSE(e.id.value.empty());
    REQUIRE_FALSE(e.timestamp.empty());
    REQUIRE(e.type == "TestEvent");
}
