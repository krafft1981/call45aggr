#pragma once
#include <time.h>
#include <ostream>

// Специальная манипуляция для изменения жизни лога

struct log_delay_impl {
    time_t delay_ = 60;
};

template<time_t N>
log_delay_impl log_delay() {
    return log_delay_impl{N};
}
