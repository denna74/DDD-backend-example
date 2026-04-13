#pragma once
#include "shared_kernel/types.h"
#include "kitchen/domain/station.h"
#include <string>

namespace kitchen {

class Dish {
public:
    Dish(shared::Id id, std::string name, int prepTimeMinutes,
         std::string cookingMethod, Station station, bool available = true)
        : id_(std::move(id))
        , name_(std::move(name))
        , prepTimeMinutes_(prepTimeMinutes)
        , cookingMethod_(std::move(cookingMethod))
        , station_(station)
        , available_(available) {}

    const shared::Id& id() const { return id_; }
    const std::string& name() const { return name_; }
    int prepTimeMinutes() const { return prepTimeMinutes_; }
    const std::string& cookingMethod() const { return cookingMethod_; }
    Station station() const { return station_; }
    bool isAvailable() const { return available_; }

    void markOutOfStock() { available_ = false; }
    void restore() { available_ = true; }

private:
    shared::Id id_;
    std::string name_;
    int prepTimeMinutes_;
    std::string cookingMethod_;
    Station station_;
    bool available_;
};

} // namespace kitchen
