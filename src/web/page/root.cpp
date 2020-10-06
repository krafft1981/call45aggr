
#include <web/view.hpp>
#include "root.hpp"

http_response root_view::handle(const http_request &request) {
    http_response result;

    result.code_ = 301;
    result.fields_ = {{"Location", "/state"}};
    result.mime_type_ = "text/html";

    return result;
}
