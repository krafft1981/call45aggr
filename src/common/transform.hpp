#pragma once

#include <vector>
#include <algorithm>
#include "type.hpp"
#include <common/meta/accessor.hpp>
// Some transformation functions sugar

namespace transform {
    template<typename Container, typename Function>
    inline auto to_vector(Container &&cont, Function &&f) {

        using type = std::decay_t<decltype(f(*cont.begin()))>;

        std::vector<type> result;
        result.reserve(cont.size());

        for (auto &v : cont) {
            result.push_back(f(v));
        }
        return result;
    }
}

namespace member {

    template<typename Container, typename Class, typename ValueType>
    inline std::vector<ValueType> to_vector(Container &&cont, member_ptr_t <Class, ValueType> mem_ptr) {

        std::vector<ValueType> result;
        result.reserve(cont.size());

        for (auto &v : cont) {
            result.push_back(meta::access_field(v, mem_ptr));
        }
        return result;
    }

    template<typename Container, typename Class, typename ValueType>
    inline std::vector<ValueType> to_unique_vector(Container &&cont, member_ptr_t <Class, ValueType> mem_ptr) {
        auto result = to_vector(cont, mem_ptr);

        std::sort(result.begin(), result.end());
        auto it = std::unique(result.begin(), result.end());
        result.erase(it, result.end());

        return result;
    }
}
