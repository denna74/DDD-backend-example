#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <any>

namespace shared {

class EventBus {
public:
    template <typename E>
    void subscribe(std::function<void(const E&)> handler) {
        auto key = std::type_index(typeid(E));
        handlers_[key].push_back([handler = std::move(handler)](const std::any& event) {
            handler(std::any_cast<const E&>(event));
        });
    }

    template <typename E>
    void publish(const E& event) {
        auto key = std::type_index(typeid(E));
        auto it = handlers_.find(key);
        if (it != handlers_.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }

private:
    std::unordered_map<std::type_index, std::vector<std::function<void(const std::any&)>>> handlers_;
};

} // namespace shared
