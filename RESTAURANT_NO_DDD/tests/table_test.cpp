#include <catch2/catch_test_macros.hpp>
#include "db.h"
#include "repositories/table_repository.h"

static Database& testDb() {
    static Database db(":memory:");
    static bool initialized = false;
    if (!initialized) {
        db.exec(
            "CREATE TABLE IF NOT EXISTS tables ("
            "  id TEXT PRIMARY KEY,"
            "  table_number INTEGER NOT NULL,"
            "  capacity INTEGER NOT NULL,"
            "  status TEXT NOT NULL"
            ");"
        );
        initialized = true;
    }
    return db;
}

TEST_CASE("Table: create and findById") {
    auto& db = testDb();
    TableRepository repo(db);

    Table table;
    table.id           = "table-001";
    table.table_number = 1;
    table.capacity     = 4;
    table.status       = "AVAILABLE";

    repo.create(table);

    auto found = repo.findById("table-001");
    REQUIRE(found.has_value());
    REQUIRE(found->id == "table-001");
    REQUIRE(found->table_number == 1);
    REQUIRE(found->capacity == 4);
    REQUIRE(found->status == "AVAILABLE");
}

TEST_CASE("Table: findAll returns all tables") {
    auto& db = testDb();
    TableRepository repo(db);

    Table table1;
    table1.id           = "fa-table-001";
    table1.table_number = 10;
    table1.capacity     = 4;
    table1.status       = "AVAILABLE";
    repo.create(table1);

    Table table2;
    table2.id           = "fa-table-002";
    table2.table_number = 11;
    table2.capacity     = 6;
    table2.status       = "AVAILABLE";
    repo.create(table2);

    Table table3;
    table3.id           = "fa-table-003";
    table3.table_number = 12;
    table3.capacity     = 2;
    table3.status       = "OCCUPIED";
    repo.create(table3);

    auto all = repo.findAll();
    REQUIRE(all.size() >= 3);
}

TEST_CASE("Table: updateStatus") {
    auto& db = testDb();
    TableRepository repo(db);

    Table table;
    table.id           = "table-004";
    table.table_number = 4;
    table.capacity     = 8;
    table.status       = "AVAILABLE";
    repo.create(table);

    repo.updateStatus("table-004", "OCCUPIED");

    auto found = repo.findById("table-004");
    REQUIRE(found.has_value());
    REQUIRE(found->status == "OCCUPIED");

    repo.updateStatus("table-004", "AVAILABLE");
    found = repo.findById("table-004");
    REQUIRE(found->status == "AVAILABLE");
}
