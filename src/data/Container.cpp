#include <iostream>

#include "Container.hpp"
#include "BaseDataImpl.hpp"

#include "db/BDb.h"

#include "daemon/daemon.hpp"
#include "daemon/life_span.hpp"

Container::Container()
    : loaded_(false)
{}

#define NP(nm) std::make_pair(#nm, (std::shared_ptr<BaseData>)nm)
std::vector<std::pair<char const*,std::shared_ptr<BaseData>>> Container::all_data() {
    return {
    };
}
#undef NP

bool Container::loaded() {

    if (loaded_) {
        return true;
    }

    for (auto& v : all_data()){
        if (!v.second->get_base()) {
            return false;
        }
    }

    loaded_ = true;
    return true;
}

void Container::loadall() {}
