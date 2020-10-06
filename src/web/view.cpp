#include "view.hpp"
#include "common.hpp"

http_response error_response(int code, char const* reason) {
    http_response result;
    result.mime_type_ = "application/xml";
    result.code_ = code;
    result.body_ = concat("<error>", reason, "</error>");
    return result;
}

http_response bad_request(char const* reason) {
    return error_response(400, reason);
}

http_response not_found(char const* reason) {
    return error_response(404, reason);
}

http_response server_error(char const* reason) {
    return error_response(500, reason);
}