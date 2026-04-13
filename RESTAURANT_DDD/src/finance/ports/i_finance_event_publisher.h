#pragma once
#include "finance/domain/finance_events.h"

namespace finance {

class IFinanceEventPublisher {
public:
    virtual ~IFinanceEventPublisher() = default;
    virtual void publishWasteRecorded(const WasteRecorded&) = 0;
};

} // namespace finance
