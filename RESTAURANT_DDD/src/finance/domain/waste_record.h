#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace finance {

class WasteRecord {
public:
    WasteRecord(shared::Id id, shared::Id costItemId,
                double quantity, std::string unit, std::string reason,
                std::string recordedAt = "")
        : id_(std::move(id))
        , costItemId_(std::move(costItemId))
        , quantity_(quantity)
        , unit_(std::move(unit))
        , reason_(std::move(reason))
        , recordedAt_(recordedAt.empty() ? shared::Timestamp::now().value : std::move(recordedAt)) {}

    const shared::Id& id() const { return id_; }
    const shared::Id& costItemId() const { return costItemId_; }
    double quantity() const { return quantity_; }
    const std::string& unit() const { return unit_; }
    const std::string& reason() const { return reason_; }
    const std::string& recordedAt() const { return recordedAt_; }

private:
    shared::Id id_;
    shared::Id costItemId_;
    double quantity_;
    std::string unit_;
    std::string reason_;
    std::string recordedAt_;
};

} // namespace finance
