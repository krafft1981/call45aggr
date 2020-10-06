#pragma once

#include <string>
#include <ostream>
#include <sstream>

// Выводит название функции в поток
struct compact_pretty_function {
    char const* pretty_ = nullptr;
    char const* name_   = nullptr;
    compact_pretty_function(char const* pretty, char const* nm)
            : pretty_(pretty), name_(nm)
    {}

    compact_pretty_function() = default;

    explicit operator bool() const {
        return pretty_ != nullptr && name_ != nullptr;
    }

    // Находит название функции в строке __PRETTY_FUNCTION__, и выводит полное имя функции
    friend std::ostream& operator << (std::ostream& os, compact_pretty_function const& other) {

        if (other.name_ == nullptr || other.pretty_ == nullptr) {
            return os;
        }

        std::string pretty = other.pretty_;
        std::string name = other.name_;

        size_t func_place = pretty.find(name + "(");
        if (func_place == std::string::npos) {
            os << "Unknown";
            return os;
        }

        auto place = func_place;
        while (place > 0 && pretty[place-1] != ' ') {
            --place;
        }

        std::copy(pretty.begin() + place, pretty.begin() + func_place + name.size(),
                  std::ostreambuf_iterator<char>(os));

        return os;
    }


    std::string str() {
        std::ostringstream ss;
        ss << *this;
        return ss.str();
    }
};

#define FUNCTION_NAME compact_pretty_function(__PRETTY_FUNCTION__, __func__)
