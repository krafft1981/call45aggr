#pragma once

#include <vector>
#include <algorithm>

template<typename Type>
struct timed_storage {
    time_t date_start_;
    time_t date_end_;
    Type value_;
};

template<typename Type>
struct timed_vector : public std::vector<timed_storage<Type>> {

    auto find_by_start(time_t start) {
        auto it = std::lower_bound(this->begin(), this->end(), start, [](timed_storage<Type> const& v, time_t start) {
            return v.date_start_ < start;
        });
        return it;
    }

    void insert_range(time_t start, time_t finish, Type&& value) {
        auto it = find_by_start(start);
        this->insert(it, timed_storage<Type>{start, finish, value});
    }

    Type find_value(time_t time) const {
        auto it = std::upper_bound(this->begin(), this->end(), time, []( time_t t, timed_storage<Type> const& v) {
            return v.date_end_ > t;
        });

        if (it == this->end() || it->date_end_ < time ){
            return Type{};
        }

        return it->value_;
    }
};
