#pragma once

#include <variant>
#include <unordered_map>
#include <json/json.h>

class parameters : public std::unordered_map<std::string, std::variant<double, std::string, int>> {
public:
    parameters(std::initializer_list<value_type> l)
            : unordered_map(l)
    {}

    void dump(Json::Value& val) const {
        for (auto& p : *this){
            std::visit([&](auto&& arg){
                val[p.first] = arg;
            }, p.second);
        }
    }
};

inline std::ostream& operator << (std::ostream& os, parameters const& ){
    return os;
}
