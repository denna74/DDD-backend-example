#pragma once
#include "floor/domain/table.h"
#include "floor/domain/menu_item.h"
#include "floor/domain/seating.h"
#include <vector>
#include <optional>

namespace floor_ctx {

class IFloorRepository {
public:
    virtual ~IFloorRepository() = default;

    virtual void saveTable(const Table& table) = 0;
    virtual void updateTable(const Table& table) = 0;
    virtual std::optional<Table> findTableById(const shared::Id& id) = 0;
    virtual std::vector<Table> findAllTables() = 0;

    virtual void saveMenuItem(const MenuItem& item) = 0;
    virtual void updateMenuItem(const MenuItem& item) = 0;
    virtual std::optional<MenuItem> findMenuItemById(const shared::Id& id) = 0;
    virtual std::optional<MenuItem> findMenuItemByName(const std::string& name) = 0;
    virtual std::vector<MenuItem> findAllMenuItems() = 0;

    virtual void saveSeating(const Seating& seating) = 0;
    virtual void updateSeating(const Seating& seating) = 0;
    virtual std::optional<Seating> findActiveSeatingByTableId(const shared::Id& tableId) = 0;
    virtual int countCoversToday() = 0;
};

} // namespace floor_ctx
