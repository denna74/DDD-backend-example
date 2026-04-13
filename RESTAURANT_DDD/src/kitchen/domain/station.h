#pragma once
#include <string>
#include <stdexcept>

namespace kitchen {
enum class Station { Grill, Sauce, Cold, Pastry };

inline std::string stationToString(Station s) {
    switch (s) {
        case Station::Grill: return "grill";
        case Station::Sauce: return "sauce";
        case Station::Cold: return "cold";
        case Station::Pastry: return "pastry";
    }
    return "unknown";
}

inline Station stationFromString(const std::string& s) {
    if (s == "grill") return Station::Grill;
    if (s == "sauce") return Station::Sauce;
    if (s == "cold") return Station::Cold;
    if (s == "pastry") return Station::Pastry;
    throw std::invalid_argument("Unknown station: " + s);
}
} // namespace kitchen
