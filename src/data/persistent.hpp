#pragma once

#include <string>

void store_parameter(char const* name, std::string const& value);
void store_parameter(char const* name, long long int value);

template<typename T>
T  load_parameter(char const* name);
