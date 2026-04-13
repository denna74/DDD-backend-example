#include <catch2/catch_test_macros.hpp>
#include "db.h"
#include "repositories/reservation_repository.h"

static Database& testDb() {
    static Database db(":memory:");
    static bool initialized = false;
    if (!initialized) {
        // Note: table is named "seatings" — reservation repo maps this to Reservation model
        db.exec(
            "CREATE TABLE IF NOT EXISTS seatings ("
            "  id TEXT PRIMARY KEY,"
            "  table_id TEXT NOT NULL,"
            "  cover_count INTEGER NOT NULL DEFAULT 0,"
            "  is_walk_in INTEGER NOT NULL DEFAULT 0,"
            "  reservation_name TEXT,"
            "  seated_at TEXT,"
            "  cleared_at TEXT"
            ");"
        );
        initialized = true;
    }
    return db;
}

TEST_CASE("Reservation: create UNPLANNED (walk-in) and findById") {
    auto& db = testDb();
    ReservationRepository repo(db);

    Reservation r;
    r.id               = "res-001";
    r.table_id         = "table-001";
    r.guest_count      = 3;   // Sara calls these "covers"
    r.type             = "UNPLANNED";  // Sara calls this a "walk-in"
    r.seated_at        = "2026-04-10T18:30:00";

    repo.create(r);

    auto found = repo.findById("res-001");
    REQUIRE(found.has_value());
    REQUIRE(found->id == "res-001");
    REQUIRE(found->table_id == "table-001");
    REQUIRE(found->guest_count == 3);
    REQUIRE(found->type == "UNPLANNED");
    REQUIRE(found->seated_at == "2026-04-10T18:30:00");
    REQUIRE(found->cleared_at.empty());
}

TEST_CASE("Reservation: create RESERVED and findById") {
    auto& db = testDb();
    ReservationRepository repo(db);

    Reservation r;
    r.id               = "res-002";
    r.table_id         = "table-002";
    r.guest_count      = 4;
    r.type             = "RESERVED";
    r.reservation_name = "Johnson";
    r.seated_at        = "2026-04-10T19:00:00";

    repo.create(r);

    auto found = repo.findById("res-002");
    REQUIRE(found.has_value());
    REQUIRE(found->type == "RESERVED");
    REQUIRE(found->reservation_name == "Johnson");
    REQUIRE(found->guest_count == 4);
}

TEST_CASE("Reservation: update (mark cleared)") {
    auto& db = testDb();
    ReservationRepository repo(db);

    Reservation r;
    r.id          = "res-003";
    r.table_id    = "table-003";
    r.guest_count = 2;
    r.type        = "UNPLANNED";
    r.seated_at   = "2026-04-10T20:00:00";
    repo.create(r);

    repo.update("res-003", "2026-04-10T21:45:00");

    auto found = repo.findById("res-003");
    REQUIRE(found.has_value());
    REQUIRE(found->cleared_at == "2026-04-10T21:45:00");
}

TEST_CASE("Reservation: findAll returns all reservations") {
    auto& db = testDb();
    ReservationRepository repo(db);

    Reservation r1;
    r1.id          = "fa-res-001";
    r1.table_id    = "table-001";
    r1.guest_count = 2;
    r1.type        = "UNPLANNED";
    r1.seated_at   = "2026-04-10T17:00:00";
    repo.create(r1);

    Reservation r2;
    r2.id               = "fa-res-002";
    r2.table_id         = "table-002";
    r2.guest_count      = 4;
    r2.type             = "RESERVED";
    r2.reservation_name = "Smith";
    r2.seated_at        = "2026-04-10T17:30:00";
    repo.create(r2);

    Reservation r3;
    r3.id          = "fa-res-003";
    r3.table_id    = "table-003";
    r3.guest_count = 3;
    r3.type        = "UNPLANNED";
    r3.seated_at   = "2026-04-10T18:00:00";
    repo.create(r3);

    auto all = repo.findAll();
    REQUIRE(all.size() >= 3);

    bool foundUnplanned = false;
    bool foundReserved  = false;
    for (const auto& r : all) {
        if (r.type == "UNPLANNED") foundUnplanned = true;
        if (r.type == "RESERVED")  foundReserved  = true;
    }
    REQUIRE(foundUnplanned);
    REQUIRE(foundReserved);
}
