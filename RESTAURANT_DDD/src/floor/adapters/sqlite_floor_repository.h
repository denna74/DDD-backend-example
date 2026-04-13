#pragma once
#include "floor/ports/i_floor_repository.h"
#include "shared_kernel/db.h"

namespace floor_ctx {

class SqliteFloorRepository : public IFloorRepository {
public:
    explicit SqliteFloorRepository(Database& db);

    void saveTable(const Table& table) override;
    void updateTable(const Table& table) override;
    std::optional<Table> findTableById(const shared::Id& id) override;
    std::vector<Table> findAllTables() override;

    void saveMenuItem(const MenuItem& item) override;
    void updateMenuItem(const MenuItem& item) override;
    std::optional<MenuItem> findMenuItemById(const shared::Id& id) override;
    std::optional<MenuItem> findMenuItemByName(const std::string& name) override;
    std::vector<MenuItem> findAllMenuItems() override;

    void saveSeating(const Seating& seating) override;
    void updateSeating(const Seating& seating) override;
    std::optional<Seating> findActiveSeatingByTableId(const shared::Id& tableId) override;
    int countCoversToday() override;

private:
    Database& db_;
};

} // namespace floor_ctx
