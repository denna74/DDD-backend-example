#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace kitchen {

enum class FireLineStatus { Waiting, Fired, Plated };

inline std::string fireLineStatusToString(FireLineStatus s) {
    switch (s) {
        case FireLineStatus::Waiting: return "waiting";
        case FireLineStatus::Fired: return "fired";
        case FireLineStatus::Plated: return "plated";
    }
    return "unknown";
}

inline FireLineStatus fireLineStatusFromString(const std::string& s) {
    if (s == "waiting") return FireLineStatus::Waiting;
    if (s == "fired") return FireLineStatus::Fired;
    if (s == "plated") return FireLineStatus::Plated;
    throw std::invalid_argument("Unknown fire line status: " + s);
}

class FireOrderLine {
public:
    FireOrderLine(shared::Id id, shared::Id dishId, int fireAtOffsetMinutes)
        : id_(std::move(id))
        , dishId_(std::move(dishId))
        , fireAtOffsetMinutes_(fireAtOffsetMinutes)
        , status_(FireLineStatus::Waiting) {}

    const shared::Id& id() const { return id_; }
    const shared::Id& dishId() const { return dishId_; }
    int fireAtOffsetMinutes() const { return fireAtOffsetMinutes_; }
    FireLineStatus status() const { return status_; }
    const shared::Timestamp& firedAt() const { return firedAt_; }
    const shared::Timestamp& platedAt() const { return platedAt_; }

    void fire() {
        status_ = FireLineStatus::Fired;
        firedAt_ = shared::Timestamp::now();
    }

    void plate() {
        status_ = FireLineStatus::Plated;
        platedAt_ = shared::Timestamp::now();
    }

private:
    shared::Id id_;
    shared::Id dishId_;
    int fireAtOffsetMinutes_;
    FireLineStatus status_;
    shared::Timestamp firedAt_;
    shared::Timestamp platedAt_;
};

} // namespace kitchen
