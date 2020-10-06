#include "log_level.hpp"

log_message_level global_log_level = log_message_level ::DEBUG;

// global log level
log_message_level log_level() {
    return global_log_level;
}

void set_log_level(log_message_level level){
    global_log_level = level;
}
