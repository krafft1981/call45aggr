#pragma once

#include <web/view.hpp>
#include <memory>

// web-server view
struct healthcheck_view : public view {
    virtual http_response handle(http_request const& request) override;
};
