#pragma once
#include <string>
#include <map>
#include <functional>

class ActionHandler {
public:
    using HandlerFunc = std::function<std::string(const std::string&)>;

    // Register a handler for a topic
    void register_action_handler(const std::string& topic, HandlerFunc handler) {
        action_handlers_[topic] = handler;
    }

    // Run the handler for a topic, returns pair<found, result>
    std::pair<bool, std::string> run_handler(const std::string& topic, const std::string& payload) const {
        auto it = action_handlers_.find(topic);
        if (it != action_handlers_.end()) {
            return {true, it->second(payload)};
        } else {
            return {false, "No handler for action: " + topic};
        }
    }

private:
    std::map<std::string, HandlerFunc> action_handlers_;
}; 