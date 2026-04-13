#include "finance/application/finance_service.h"

namespace finance {

FinanceService::FinanceService(IFinanceRepository& repo, IFinanceEventPublisher& events)
    : repo_(repo), events_(events) {}

std::vector<CostItem> FinanceService::listCostItems() {
    return repo_.findAllCostItems();
}

std::string FinanceService::addCostItem(const std::string& name, int ingredientCostCents,
                                         int supplierPriceCents, int sellingPriceCents) {
    auto id = shared::Id::generate();
    CostItem item(id, name,
                  shared::Money(ingredientCostCents),
                  shared::Money(supplierPriceCents),
                  shared::Money(sellingPriceCents));
    repo_.saveCostItem(item);
    return id.value;
}

shared::Result<void> FinanceService::updateCostItem(const std::string& id,
                                                      int ingredientCostCents,
                                                      int supplierPriceCents) {
    auto item = repo_.findCostItemById(shared::Id{id});
    if (!item) {
        return shared::Result<void>::fail("Cost item not found");
    }
    item->updateIngredientCost(shared::Money(ingredientCostCents));
    item->updateSupplierPrice(shared::Money(supplierPriceCents));
    repo_.updateCostItem(*item);
    return shared::Result<void>::ok();
}

std::optional<double> FinanceService::calculateMargin(const std::string& id) {
    auto item = repo_.findCostItemById(shared::Id{id});
    if (!item) {
        return std::nullopt;
    }
    return item->marginPercent();
}

std::string FinanceService::recordWaste(const std::string& costItemId, double quantity,
                                         const std::string& unit, const std::string& reason) {
    auto id = shared::Id::generate();
    WasteRecord record(id, shared::Id{costItemId}, quantity, unit, reason);
    repo_.saveWasteRecord(record);
    events_.publishWasteRecorded(
        WasteRecorded(shared::Id{costItemId}, quantity, unit, reason));
    return id.value;
}

std::vector<WasteRecord> FinanceService::listWaste() {
    return repo_.findAllWasteRecords();
}

FoodCostRatio FinanceService::foodCostReport() {
    return repo_.calculateFoodCostRatio();
}

} // namespace finance
