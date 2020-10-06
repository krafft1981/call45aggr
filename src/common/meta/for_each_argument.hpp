#pragma once
#include <utility>

template<typename Fun, typename...T>
inline void for_each_argument(Fun&& fun, T&&... args) {
    (fun (std::forward<T>(args)), ...);
}
