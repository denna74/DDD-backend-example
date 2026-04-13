#pragma once
#include "finance/ports/i_finance_repository.h"
#include "finance/ports/i_finance_event_publisher.h"
#include "shared_kernel/types.h"
#include <string>
#include <vector>
#include <optional>

namespace finance {

class FinanceService {
public:
    FinanceService(IFinanceRepository& repo, IFinanceEventPublisher& events);

    std::vector<CostItem> listCostItems();
    std::string addCostItem(const std::string& name, int ingredientCostCents,
                            int supplierPriceCents, int sellingPriceCents);
    shared::Result<void> updateCostItem(const std::string& id,
                                         int ingredientCostCents, int supplierPriceCents);
    std::optional<double> calculateMargin(const std::string& id);

    std::string recordWaste(const std::string& costItemId, double quantity,
                            const std::string& unit, const std::string& reason);
    std::vector<WasteRecord> listWaste();

    FoodCostRatio foodCostReport();

private:
    IFinanceRepository& repo_;
    IFinanceEventPublisher& events_;
};

} // namespace finance
