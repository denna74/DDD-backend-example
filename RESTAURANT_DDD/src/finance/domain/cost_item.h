#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace finance {

class CostItem {
public:
    CostItem(shared::Id id, std::string name,
             shared::Money ingredientCost, shared::Money supplierPrice, shared::Money sellingPrice)
        : id_(std::move(id))
        , name_(std::move(name))
        , ingredientCost_(ingredientCost)
        , supplierPrice_(supplierPrice)
        , sellingPrice_(sellingPrice) {}

    const shared::Id& id() const { return id_; }
    const std::string& name() const { return name_; }
    shared::Money ingredientCost() const { return ingredientCost_; }
    shared::Money supplierPrice() const { return supplierPrice_; }
    shared::Money sellingPrice() const { return sellingPrice_; }

    double marginPercent() const {
        if (sellingPrice_.cents == 0) return 0.0;
        return static_cast<double>(sellingPrice_.cents - ingredientCost_.cents)
               / sellingPrice_.cents * 100.0;
    }

    void updateSupplierPrice(shared::Money price) { supplierPrice_ = price; }
    void updateIngredientCost(shared::Money cost) { ingredientCost_ = cost; }

private:
    shared::Id id_;
    std::string name_;
    shared::Money ingredientCost_;
    shared::Money supplierPrice_;
    shared::Money sellingPrice_;
};

} // namespace finance
