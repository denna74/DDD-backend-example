#pragma once
#include "finance/domain/cost_item.h"
#include "finance/domain/waste_record.h"
#include "finance/domain/food_cost_ratio.h"
#include <vector>
#include <optional>

namespace finance {

class IFinanceRepository {
public:
    virtual ~IFinanceRepository() = default;
    virtual void saveCostItem(const CostItem& item) = 0;
    virtual void updateCostItem(const CostItem& item) = 0;
    virtual std::optional<CostItem> findCostItemById(const shared::Id& id) = 0;
    virtual std::vector<CostItem> findAllCostItems() = 0;
    virtual void saveWasteRecord(const WasteRecord& record) = 0;
    virtual std::vector<WasteRecord> findAllWasteRecords() = 0;
    virtual std::vector<WasteRecord> findWasteRecordsByDateRange(const std::string& start, const std::string& end) = 0;
    virtual FoodCostRatio calculateFoodCostRatio() = 0;
};

} // namespace finance
