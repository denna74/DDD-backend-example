#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace floor_ctx {

// MenuItem — Sara's view of an item.
// She sees name, description, price, and availability.
// No prep_time, no station, no costs — that's Marco's and Diane's world.

class MenuItem {
public:
    MenuItem(shared::Id id, std::string name, std::string description,
             shared::Money price, bool available = true)
        : id_(std::move(id))
        , name_(std::move(name))
        , description_(std::move(description))
        , price_(price)
        , available_(available) {}

    const shared::Id& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& description() const { return description_; }
    shared::Money price() const { return price_; }
    bool isAvailable() const { return available_; }

    void markSoldOut() { available_ = false; }
    void restore() { available_ = true; }

private:
    shared::Id id_;
    std::string name_;
    std::string description_;
    shared::Money price_;
    bool available_;
};

} // namespace floor_ctx
