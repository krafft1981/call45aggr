#pragma once
#include <map>

void start_data_loader();
// Ошибки загрузки данных
// Возвращаемое значение event->error
std::map<std::string, std::string> data_load_errors();
