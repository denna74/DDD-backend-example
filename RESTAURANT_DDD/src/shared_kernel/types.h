#pragma once

#include <string>
#include <optional>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace shared {

struct Id {
    std::string value;

    static Id generate() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        const char hex[] = "0123456789abcdef";
        std::string result;
        result.reserve(32);
        for (int i = 0; i < 32; i++) {
            result += hex[dis(gen)];
        }
        return Id{result};
    }

    bool operator==(const Id& other) const { return value == other.value; }
    bool operator!=(const Id& other) const { return value != other.value; }
};

struct Money {
    int cents;

    Money() : cents(0) {}
    explicit Money(int c) : cents(c) {}

    Money operator+(const Money& other) const { return Money(cents + other.cents); }
    Money operator-(const Money& other) const { return Money(cents - other.cents); }
    bool operator==(const Money& other) const { return cents == other.cents; }
    bool operator!=(const Money& other) const { return cents != other.cents; }

    double toDollars() const { return cents / 100.0; }
};

struct Timestamp {
    std::string value;

    static Timestamp now() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        gmtime_r(&time_t, &tm_buf);
        std::ostringstream ss;
        ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
        return Timestamp{ss.str()};
    }

    bool empty() const { return value.empty(); }
};

template <typename T>
struct Result {
    std::optional<T> value;
    std::string error;

    static Result<T> ok(T val) { return Result<T>{std::move(val), ""}; }
    static Result<T> fail(std::string err) { return Result<T>{std::nullopt, std::move(err)}; }

    bool isOk() const { return value.has_value(); }
    bool isError() const { return !value.has_value(); }
};

template <>
struct Result<void> {
    bool success;
    std::string error;

    static Result<void> ok() { return Result<void>{true, ""}; }
    static Result<void> fail(std::string err) { return Result<void>{false, std::move(err)}; }

    bool isOk() const { return success; }
    bool isError() const { return !success; }
};

} // namespace shared
