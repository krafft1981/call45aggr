#pragma once
#include <ostream>

struct Id {
    int64_t id_;
    Id(int64_t i) : id_(i){}

    friend std::ostream& operator << (std::ostream& os, Id const& i) {
        if (i.id_ != 0){
            os << i.id_;
        } else {
            os << "NULL";
        }
        return os;
    }
};
