#pragma once
#include <vector>
#include "log_message.hpp"

struct log_handler {
    // Коллбеки для разных типов обработки
    virtual void on_log(std::vector<log_message_ptr> const &messages) {
    }

    virtual ~log_handler() {}
};
