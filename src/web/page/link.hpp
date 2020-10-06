#pragma once

#include "web/view.hpp"

struct link_view : public view {
    virtual http_response handle(http_request const &request) override ;
};
