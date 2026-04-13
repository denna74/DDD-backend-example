#pragma once

#include "shared_kernel/types.h"
#include <string>

namespace shared {

struct DomainEvent {
    Id id;
    std::string timestamp;
    std::string type;

    explicit DomainEvent(std::string eventType)
        : id(Id::generate())
        , timestamp(Timestamp::now().value)
        , type(std::move(eventType)) {}

    virtual ~DomainEvent() = default;
};

} // namespace shared
