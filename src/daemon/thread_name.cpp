#include "thread_name.hpp"

thread_local std::string thread_name;

// Имя потока нужно для однозначной идентификации потока
void set_current_thread_name(std::string const& name) {
    thread_name = name;
}

std::string const& current_thread_name() {
    return thread_name;
}
