#include <thread>
#include "log.hpp"
#include "log_instance.hpp"
#include "daemon/thread_name.hpp"

namespace Log {
    void send(log_message &&message) {
        message.thread_name = current_thread_name();
        send_log_message(std::move(message));
    }
} // namespace log
