#pragma once
#include "shared_kernel/domain_event.h"
#include <string>

namespace finance {

struct WasteRecorded : shared::DomainEvent {
    shared::Id costItemId;
    double quantity;
    std::string unit;
    std::string reason;

    WasteRecorded(shared::Id c, double q, std::string u, std::string r)
        : DomainEvent("WasteRecorded")
        , costItemId(std::move(c))
        , quantity(q)
        , unit(std::move(u))
        , reason(std::move(r)) {}
};

} // namespace finance
