#pragma once

#include <string>

// Имя потока нужно для однозначной идентификации потока
void set_current_thread_name(std::string const& name);
std::string const& current_thread_name();
