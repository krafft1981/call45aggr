#pragma once

#include <vector>
#include <list>
#include <set>
#include "meta/accessor.hpp"
#include "array_view.hpp"

template<typename Container>
inline void dump_container(std::ostream& os, Container const & container) {
    os << "{";
    for (auto const& value : container ){
        if (&value != &*container.begin()){
            os << ",";
        }
        os << meta::access_value(value);
    }
    os << "}";
}

template<typename T> inline std::ostream& operator << (std::ostream& os, std::vector<T> const& c) { dump_container(os,c); return os; }
template<typename T> inline std::ostream& operator << (std::ostream& os, std::set<T> const& c) { dump_container(os,c); return os; }
template<typename T> inline std::ostream& operator << (std::ostream& os, std::list<T> const& c) { dump_container(os,c); return os;}
template<typename T> inline std::ostream& operator << (std::ostream& os, array_view<T> const& c) { dump_container(os,c); return os;}
