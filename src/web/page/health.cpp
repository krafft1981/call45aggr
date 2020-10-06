
#include <web/view.hpp>
#include "health.hpp"
#include "rendering.hpp"
#include <common/string_streambuf.hpp>

http_response health_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            os(&sb);

    render_header(os, "Health");

    os <<  "<pre>Not implemented</pre>";

    return resp;
}
