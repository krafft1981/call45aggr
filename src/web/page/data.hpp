#pragma once

#include "web/view.hpp"

struct data_view : public view {
    virtual http_response handle(http_request const &request) override ;
};
