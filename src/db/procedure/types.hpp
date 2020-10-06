
#pragma once
#include <ostream>
#include <stdint.h>
#include <string>

// Распространенные имена типов.
namespace procedure {

struct integer_impl {
    int64_t     value_;
    char const *name_;

    integer_impl(int64_t value, char const *name)
        : value_(value), name_(name) {}

    friend std::ostream &operator<<(std::ostream &      os,
                                    integer_impl const &impl) {
        os << impl.value_ << "::" << impl.name_;
        return os;
    }
};

struct string_impl {
    std::string const &value_;
    char const *       name_;

    string_impl(std::string const &value, char const *name)
        : value_(value), name_(name) {}

    friend std::ostream &operator<<(std::ostream &os, string_impl const &impl) {
        os << "'" << impl.value_ << "'"
           << "::" << impl.name_;
        return os;
    }
};

inline integer_impl smallint(int64_t i) { return integer_impl(i, "smallint"); }
inline integer_impl bigint(int64_t i) { return integer_impl(i, "bigint"); }
inline integer_impl integer(int64_t i) { return integer_impl(i, "integer"); }

inline string_impl text(std::string const &src) { return string_impl(src, "text"); }
inline string_impl inet(std::string const &src) { return string_impl(src, "inet"); }
} // namespace procedure
