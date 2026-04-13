#pragma once
#include "floor/ports/i_floor_repository.h"
#include "floor/ports/i_floor_event_publisher.h"
#include "shared_kernel/types.h"
#include <string>
#include <vector>

namespace floor_ctx {

class FloorService {
public:
    FloorService(IFloorRepository& repo, IFloorEventPublisher& events);

    std::vector<Table> listTables();
    std::string addTable(int tableNumber, int capacity);

    shared::Result<void> seatWalkIn(const std::string& tableId, int partySize);
    shared::Result<void> seatReservation(const std::string& tableId, int partySize,
                                         const std::string& name);
    shared::Result<void> turnTable(const std::string& tableId);

    std::vector<MenuItem> listMenuItems();
    std::string addMenuItem(const std::string& name, const std::string& description,
                            int priceCents);
    shared::Result<void> updateMenuItem(const std::string& id, const std::string& name,
                                        const std::string& description, int priceCents);
    shared::Result<void> markSoldOut(const std::string& menuItemId);
    shared::Result<void> markSoldOutByName(const std::string& name);

    int countCoversTonight();

private:
    IFloorRepository& repo_;
    IFloorEventPublisher& events_;
};

} // namespace floor_ctx
