#pragma once

#include "size_for_output.hpp"
#include "string_streambuf.hpp"
#include <ostream>


template <typename... Args>
std::string concat(Args&&... args) {
    std::string result;
    size_t      result_size = 0;

    // compute output string size
    using swallow = int[];
    (void)swallow{(result_size += utils::size_for_output(args), 0)...};

    // allocate buffer for result
    result.reserve(result_size);

    utils::string_streambuf sb(result);
    std::ostream            os(&sb);

    (void)swallow{(os << std::forward<Args>(args), 0)...};

    return result;
}
