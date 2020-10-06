//
// Created by root on 12/25/19.
//

#include <string>
#include "daemon/daemon.hpp"
#include "db/BDb.h"

void store_parameter(char const *name, const std::string &value) {
    // По сути это все можно заменить на upsert
    auto conn = app::source_conn();
    try {
        conn->exec2("INSERT INTO public.setting(name, value) VALUES('",name,"' ,'",value,"')");
    } catch (...) {
        conn->exec2("UPDATE public.setting SET value = '",value,"' WHERE name = '", name, "'" );
    }
}

void store_parameter(char const *name, long long int value) {
    store_parameter(name, std::to_string(value));
}

template<typename T>
T convert(char const* src) {
    return T(src);
}

template<>
long long int convert<long long int> (char const* src) {
    return std::strtoll(src,nullptr,10);
}

template<typename T>
T load_parameter(char const *name) {
    auto conn = app::source_conn();
    try {
        auto q = conn->query2("SELECT value FROM public.setting WHERE name = '", name, "'");
        if (!q.next()) {
            return T{};
        }
        return convert<T>(q.get(0));
    } catch (...) {
        return T{};
    }
}

template long long load_parameter<long long>(char const* name);
