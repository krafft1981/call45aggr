#pragma once

#include "web/view.hpp"

struct time_view : public view {
    http_response handle(http_request const &request) override ;
};
