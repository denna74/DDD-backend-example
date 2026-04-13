#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace floor_ctx {

class Seating {
public:
    // Static factories — Sara's language
    static Seating walkIn(shared::Id id, shared::Id tableId, int coverCount) {
        return Seating(std::move(id), std::move(tableId), coverCount,
                       true, "", shared::Timestamp::now(), shared::Timestamp{""});
    }

    static Seating reservation(shared::Id id, shared::Id tableId,
                               int coverCount, std::string name) {
        return Seating(std::move(id), std::move(tableId), coverCount,
                       false, std::move(name), shared::Timestamp::now(), shared::Timestamp{""});
    }

    // Full constructor for DB reconstruction
    Seating(shared::Id id, shared::Id tableId, int coverCount,
            bool isWalkIn, std::string reservationName,
            shared::Timestamp seatedAt, shared::Timestamp clearedAt)
        : id_(std::move(id))
        , tableId_(std::move(tableId))
        , coverCount_(coverCount)
        , isWalkIn_(isWalkIn)
        , reservationName_(std::move(reservationName))
        , seatedAt_(std::move(seatedAt))
        , clearedAt_(std::move(clearedAt)) {}

    const shared::Id& id() const { return id_; }
    const shared::Id& tableId() const { return tableId_; }
    int coverCount() const { return coverCount_; }
    bool isWalkIn() const { return isWalkIn_; }
    const std::string& reservationName() const { return reservationName_; }
    const shared::Timestamp& seatedAt() const { return seatedAt_; }
    const shared::Timestamp& clearedAt() const { return clearedAt_; }

    void clear() {
        clearedAt_ = shared::Timestamp::now();
    }

private:
    shared::Id id_;
    shared::Id tableId_;
    int coverCount_;
    bool isWalkIn_;
    std::string reservationName_;
    shared::Timestamp seatedAt_;
    shared::Timestamp clearedAt_;
};

} // namespace floor_ctx
