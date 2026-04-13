#pragma once
#include "shared_kernel/types.h"
#include <string>

namespace floor_ctx {

enum class TableStatus { Available, Occupied, Reserved };

inline std::string tableStatusToString(TableStatus s) {
    switch (s) {
        case TableStatus::Available: return "available";
        case TableStatus::Occupied:  return "occupied";
        case TableStatus::Reserved:  return "reserved";
    }
    return "unknown";
}

inline TableStatus tableStatusFromString(const std::string& s) {
    if (s == "available") return TableStatus::Available;
    if (s == "occupied")  return TableStatus::Occupied;
    if (s == "reserved")  return TableStatus::Reserved;
    throw std::invalid_argument("Unknown table status: " + s);
}

class Table {
public:
    Table(shared::Id id, int tableNumber, int capacity,
          TableStatus status = TableStatus::Available)
        : id_(std::move(id))
        , tableNumber_(tableNumber)
        , capacity_(capacity)
        , status_(status) {}

    const shared::Id& id() const { return id_; }
    int tableNumber() const { return tableNumber_; }
    int capacity() const { return capacity_; }
    TableStatus status() const { return status_; }

    shared::Result<void> occupy() {
        if (status_ != TableStatus::Available && status_ != TableStatus::Reserved) {
            return shared::Result<void>::fail("Table is not available or reserved");
        }
        status_ = TableStatus::Occupied;
        return shared::Result<void>::ok();
    }

    void turn() {
        status_ = TableStatus::Available;
    }

    void reserve() {
        status_ = TableStatus::Reserved;
    }

private:
    shared::Id id_;
    int tableNumber_;
    int capacity_;
    TableStatus status_;
};

} // namespace floor_ctx
