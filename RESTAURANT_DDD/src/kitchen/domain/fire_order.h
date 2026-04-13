#pragma once
#include "shared_kernel/types.h"
#include "kitchen/domain/fire_order_line.h"
#include <vector>
#include <string>
#include <algorithm>

namespace kitchen {

enum class FireOrderStatus { Coordinating, InProgress, Plated };

inline std::string fireOrderStatusToString(FireOrderStatus s) {
    switch (s) {
        case FireOrderStatus::Coordinating: return "coordinating";
        case FireOrderStatus::InProgress: return "in_progress";
        case FireOrderStatus::Plated: return "plated";
    }
    return "unknown";
}

inline FireOrderStatus fireOrderStatusFromString(const std::string& s) {
    if (s == "coordinating") return FireOrderStatus::Coordinating;
    if (s == "in_progress") return FireOrderStatus::InProgress;
    if (s == "plated") return FireOrderStatus::Plated;
    throw std::invalid_argument("Unknown fire order status: " + s);
}

class FireOrder {
public:
    FireOrder(shared::Id id, int tableNumber, shared::Timestamp createdAt)
        : id_(std::move(id))
        , tableNumber_(tableNumber)
        , createdAt_(std::move(createdAt))
        , status_(FireOrderStatus::Coordinating) {}

    const shared::Id& id() const { return id_; }
    int tableNumber() const { return tableNumber_; }
    const shared::Timestamp& createdAt() const { return createdAt_; }
    FireOrderStatus status() const { return status_; }
    const std::vector<FireOrderLine>& lines() const { return lines_; }

    void addLine(shared::Id lineId, shared::Id dishId, int fireAtOffsetMinutes) {
        lines_.emplace_back(std::move(lineId), std::move(dishId), fireAtOffsetMinutes);
    }

    shared::Result<void> fireDish(const shared::Id& lineId) {
        auto it = findLine(lineId);
        if (it == lines_.end()) {
            return shared::Result<void>::fail("Line not found: " + lineId.value);
        }
        if (it->status() == FireLineStatus::Fired || it->status() == FireLineStatus::Plated) {
            return shared::Result<void>::fail("Dish already fired");
        }
        it->fire();
        status_ = FireOrderStatus::InProgress;
        return shared::Result<void>::ok();
    }

    shared::Result<void> plateDish(const shared::Id& lineId) {
        auto it = findLine(lineId);
        if (it == lines_.end()) {
            return shared::Result<void>::fail("Line not found: " + lineId.value);
        }
        if (it->status() != FireLineStatus::Fired) {
            return shared::Result<void>::fail("Dish has not been fired yet");
        }
        it->plate();
        if (allPlated()) {
            status_ = FireOrderStatus::Plated;
        }
        return shared::Result<void>::ok();
    }

    bool allPlated() const {
        return !lines_.empty() && std::all_of(lines_.begin(), lines_.end(),
            [](const FireOrderLine& l) { return l.status() == FireLineStatus::Plated; });
    }

private:
    std::vector<FireOrderLine>::iterator findLine(const shared::Id& lineId) {
        return std::find_if(lines_.begin(), lines_.end(),
            [&](const FireOrderLine& l) { return l.id() == lineId; });
    }

    shared::Id id_;
    int tableNumber_;
    shared::Timestamp createdAt_;
    FireOrderStatus status_;
    std::vector<FireOrderLine> lines_;
};

} // namespace kitchen
