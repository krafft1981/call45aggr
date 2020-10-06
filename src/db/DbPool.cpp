#include "DbPool.hpp"


DbPool::DbPool(std::string const& connection_string)
        : connection_string_(connection_string) {
}

void DbPool::set_connection_string(std::string const& connection_string) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_string_ == connection_string) {
        return;
    }
    connection_string_ = connection_string;
    connections_.clear();
}

std::shared_ptr<BDb> DbPool::get() {

    std::lock_guard<std::mutex> lock(mutex_);
    if (connections_.empty()) {
         return std::make_shared<BDb>(connection_string_);
    }

    // Первое соединение самое старое
    auto it = connections_.begin();
    auto retval = it->second;
    connections_.erase(it);
    return retval;
}

// Вернуть соединение в пул
void DbPool::back(std::shared_ptr<BDb> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections_[time(nullptr)] = conn;
}

// Удалить старые соединения
void DbPool::delete_older_then(time_t threshold) {

    auto current_time = time(nullptr);

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = connections_.begin(); it != connections_.end(); ++it){
        if (current_time - it->first < threshold) {
            return;
        }
        
        connections_.erase(it);
    }
}

bool DbPool::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.empty();
}
