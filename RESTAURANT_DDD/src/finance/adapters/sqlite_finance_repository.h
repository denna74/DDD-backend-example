#pragma once
#include "finance/ports/i_finance_repository.h"
#include "shared_kernel/db.h"

namespace finance {

class SqliteFinanceRepository : public IFinanceRepository {
public:
    explicit SqliteFinanceRepository(Database& db);

    void saveCostItem(const CostItem& item) override;
    void updateCostItem(const CostItem& item) override;
    std::optional<CostItem> findCostItemById(const shared::Id& id) override;
    std::vector<CostItem> findAllCostItems() override;

    void saveWasteRecord(const WasteRecord& record) override;
    std::vector<WasteRecord> findAllWasteRecords() override;
    std::vector<WasteRecord> findWasteRecordsByDateRange(const std::string& start, const std::string& end) override;

    FoodCostRatio calculateFoodCostRatio() override;

private:
    Database& db_;
};

} // namespace finance
