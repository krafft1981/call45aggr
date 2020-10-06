#pragma once

#include <unordered_map>
#include "web/view.hpp"
#include "common.hpp"

struct url_params {
    std::string name;
    std::unordered_map<std::string, std::string> params;

    std::string const& at(std::string const& name) const {
        auto it = params.find(name);
        if (it == params.end()) {
            throw std::runtime_error(concat("Unable to find parameter \"", name, "\" in url"));
        }
        return it->second;
    }
};

inline url_params parse_path(http_request const& req) {
    url_params result;

    unsigned long position = req.path_.find('?');
    result.name = req.path_.substr(1,position-1);

    while (position != boost::string_view::npos) {
        unsigned long eq_pos = req.path_.find('=',position);
        unsigned long nval_pos = req.path_.find('&', eq_pos);
        if (nval_pos == eq_pos) {
            break;
        }
        std::string name = req.path_.substr(position + 1, eq_pos - position - 1);
        std::string value = req.path_.substr(eq_pos + 1, nval_pos - eq_pos - 1);
        result.params[name] = value;
        position = nval_pos;
    }

    return result;
}
