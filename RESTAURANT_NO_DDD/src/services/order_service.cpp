#include "services/order_service.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

static std::string generateHexId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << dist(gen)
        << std::setw(16) << dist(gen);
    return oss.str();
}

static std::string nowIso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc{};
#ifdef _WIN32
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    return buf;
}

OrderService::OrderService(OrderRepository& repo) : repo_(repo) {}

std::string OrderService::generateId() { return generateHexId(); }
std::string OrderService::currentTimestamp() { return nowIso8601(); }

std::string OrderService::createOrder(int table_number, const std::vector<std::string>& item_ids) {
    Order order;
    order.id           = generateId();
    order.table_number = table_number;
    order.status       = "OPEN";
    order.created_at   = currentTimestamp();

    std::vector<OrderLine> lines;
    for (const auto& item_id : item_ids) {
        OrderLine line;
        line.id       = generateId();
        line.order_id = order.id;
        line.item_id  = item_id;
        line.status   = "PENDING";
        lines.push_back(line);
    }

    repo_.create(order, lines);
    return order.id;
}

std::vector<OrderWithLines> OrderService::getOrders() {
    return repo_.findAll();
}

std::optional<OrderWithLines> OrderService::getOrder(const std::string& id) {
    return repo_.findById(id);
}

void OrderService::updateOrderStatus(const std::string& id, const std::string& status) {
    repo_.updateStatus(id, status);
}
