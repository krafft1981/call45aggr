#include "health_register.hpp"

void health_register::warning(std::string const& item, std::string const& status_message) {
    instance()->register_message(health_status::WARNING,item,status_message);
}

void health_register::error(std::string const& item, std::string const& status_message) {
    instance()->register_message(health_status::ERROR, item, status_message);
}

void health_register::critical(std::string const& item, std::string const& status_message) {
    instance()->register_message(health_status::CRITICAL, item, status_message);
}

void health_register::register_message(health_status status_type, std::string const &item, std::string const &status_message) {
    std::lock_guard<std::mutex> lock(mutex_);
    erase_old_messages();

    // Запрет на перезапись инстанса с большим приоритетом
    if (values_.count(item) > 0 &&
        values_.at(item).status_type < status_type) {
        return;
    }

    health_state& state = values_[item];
    state.last_time = time(0);

    state.item = item;
    state.status_message = status_message;
    state.status_type = status_type;
    state.status = [status_type]() {
        switch (status_type) {
            case health_status::WARNING: return "STATUS_WARNING";
            case health_status::ERROR:   return "STATUS_ERROR";
            case health_status::CRITICAL:return "STATUS_CRITICAL";
        }
    } ();
}

void health_register::erase_old_messages() {
    auto ts = time(0) - 1800;
    for (auto it = values_.begin(); it != values_.end();) {
        if (it->second.last_time < ts) {
            it = values_.erase(it);
        } else {
            ++it;
        }
    }
}

void health_register::clear() {
    std::lock_guard<std::mutex> lock(instance()->mutex_);
    instance()->values_.clear();
}

std::vector<health_state> health_register::states() {

    std::lock_guard<std::mutex> lock(instance()->mutex_);
    instance()->erase_old_messages();

    std::vector<health_state> result;
    for (auto& [name,hs] : instance()->values_) {
        result.push_back(hs);
    }

    return result;
}
