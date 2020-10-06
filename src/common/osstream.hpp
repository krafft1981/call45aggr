#pragma once

#include "string_streambuf.hpp"

struct osstream : public std::ostream {
    std::string buffer_;
    std::string& ref_;
    utils::string_streambuf sb_;

    osstream(size_t preallocated = 10000)
            : ref_(buffer_), sb_(ref_) {
        ref_.reserve(preallocated);
        rdbuf(&sb_);
    }

    osstream(std::string& buffer, size_t preallocated = 10000)
            : ref_(buffer), sb_(ref_) {
        ref_.reserve(preallocated);
        rdbuf(&sb_);
    }
};
