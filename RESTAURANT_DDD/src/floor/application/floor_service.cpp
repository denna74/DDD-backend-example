#include "floor/application/floor_service.h"

namespace floor_ctx {

FloorService::FloorService(IFloorRepository& repo, IFloorEventPublisher& events)
    : repo_(repo), events_(events) {}

std::vector<Table> FloorService::listTables() {
    return repo_.findAllTables();
}

std::string FloorService::addTable(int tableNumber, int capacity) {
    auto id = shared::Id::generate();
    Table table(id, tableNumber, capacity);
    repo_.saveTable(table);
    return id.value;
}

shared::Result<void> FloorService::seatWalkIn(const std::string& tableId, int partySize) {
    auto table = repo_.findTableById(shared::Id{tableId});
    if (!table) {
        return shared::Result<void>::fail("Table not found");
    }

    auto occupyResult = table->occupy();
    if (occupyResult.isError()) {
        return occupyResult;
    }

    auto seatingId = shared::Id::generate();
    auto seating = Seating::walkIn(seatingId, shared::Id{tableId}, partySize);

    repo_.saveSeating(seating);
    repo_.updateTable(*table);

    events_.publishWalkInSeated(WalkInSeated(shared::Id{tableId}, partySize));
    return shared::Result<void>::ok();
}

shared::Result<void> FloorService::seatReservation(const std::string& tableId, int partySize,
                                                     const std::string& name) {
    auto table = repo_.findTableById(shared::Id{tableId});
    if (!table) {
        return shared::Result<void>::fail("Table not found");
    }

    auto occupyResult = table->occupy();
    if (occupyResult.isError()) {
        return occupyResult;
    }

    auto seatingId = shared::Id::generate();
    auto seating = Seating::reservation(seatingId, shared::Id{tableId}, partySize, name);

    repo_.saveSeating(seating);
    repo_.updateTable(*table);

    events_.publishReservationSeated(ReservationSeated(shared::Id{tableId}, name, partySize));
    return shared::Result<void>::ok();
}

shared::Result<void> FloorService::turnTable(const std::string& tableId) {
    auto table = repo_.findTableById(shared::Id{tableId});
    if (!table) {
        return shared::Result<void>::fail("Table not found");
    }

    auto seating = repo_.findActiveSeatingByTableId(shared::Id{tableId});
    int coverCount = 0;
    if (seating) {
        coverCount = seating->coverCount();
        seating->clear();
        repo_.updateSeating(*seating);
    }

    table->turn();
    repo_.updateTable(*table);

    events_.publishTableTurned(TableTurned(shared::Id{tableId}, coverCount));
    return shared::Result<void>::ok();
}

std::vector<MenuItem> FloorService::listMenuItems() {
    return repo_.findAllMenuItems();
}

std::string FloorService::addMenuItem(const std::string& name, const std::string& description,
                                       int priceCents) {
    auto id = shared::Id::generate();
    MenuItem item(id, name, description, shared::Money(priceCents));
    repo_.saveMenuItem(item);
    return id.value;
}

shared::Result<void> FloorService::updateMenuItem(const std::string& id, const std::string& name,
                                                    const std::string& description, int priceCents) {
    auto item = repo_.findMenuItemById(shared::Id{id});
    if (!item) {
        return shared::Result<void>::fail("Menu item not found");
    }
    // Reconstruct with updated fields, preserving availability
    MenuItem updated(shared::Id{id}, name, description, shared::Money(priceCents), item->isAvailable());
    repo_.updateMenuItem(updated);
    return shared::Result<void>::ok();
}

shared::Result<void> FloorService::markSoldOut(const std::string& menuItemId) {
    auto item = repo_.findMenuItemById(shared::Id{menuItemId});
    if (!item) {
        return shared::Result<void>::fail("Menu item not found");
    }
    item->markSoldOut();
    repo_.updateMenuItem(*item);
    events_.publishMenuItemSoldOut(MenuItemSoldOut(item->id(), item->name()));
    return shared::Result<void>::ok();
}

shared::Result<void> FloorService::markSoldOutByName(const std::string& name) {
    auto item = repo_.findMenuItemByName(name);
    if (!item) {
        return shared::Result<void>::fail("Menu item not found");
    }
    item->markSoldOut();
    repo_.updateMenuItem(*item);
    events_.publishMenuItemSoldOut(MenuItemSoldOut(item->id(), item->name()));
    return shared::Result<void>::ok();
}

int FloorService::countCoversTonight() {
    return repo_.countCoversToday();
}

} // namespace floor_ctx
