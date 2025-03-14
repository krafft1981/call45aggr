#include "favicon.hpp"
#include "web/view.hpp"
#include "common.hpp"

static char const* favicosrc = "AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAABXAAAAVwAAAFcAAABXAAAAV3M3WP9zN1j/czdY/3M3WP9zN1j/uEuH/wAAAFcAAABXAAAAVwAAAAAAAAAAAAAAAAAAAACKOmb/ijpm/4o6Zv+KOmb/ijpm/4o6Zv+KOmb/uEuH/7hLh5kAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACKOmb/ijpmjgAAAAAAAAAAijpm/4o6Zv+KOmb/ijpm/7hLh/8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAuEuH/wAAAAAAAAAAAAAAAIo6Zv+KOmb/ijpm/4o6Zv+KOmb/uEuHmQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALhLh/8AAAAAAAAAAAAAAACKOmaOijpm/4o6Zv+KOmb/ijpm/7hLh/8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC4S4f/uEuHngAAAAAAAAAAAAAAAIo6Zv+KOmb/ijpm/4o6Zv+4S4f/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALhLh/+4S4f/AAAAAAAAAACKOmb/czdY/3M3WP9zN1j/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACKOmb/ijpm/3M3WP9zN1j/czdY/3M3WP9zN1hlAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIo6Zo6KOmb/ijpm/4o6Zv+KOmb/ijpm/4o6Zv+KOmb/ijpm/4o6Zv+4S4eZAAAAAAAAAAAAAAAAAAAAAMjV1TiKOmb/ijpm/2ska3VrJGs0KwD//4o6Zv+KOmb/ijpm/4o6Zv+KOmb/uEuH/wAAAAAAAAAAAAAAAAAAAAC+yspAijpm/4o6Zv9rJGs0AAAAACsA//+KOmb/ayRrdWskazQrAP//ijpm/7hLh/8AAAAAAAAAAAAAAAAAAAAAAAAAAIo6Zv+KOmb/ayRrdWskazRrJGt1ijpm/2skazQAAAAAKwD//4o6Zv+4S4f/AAAAAAAAAAAAAAAAAAAAAAAAAACKOmaOijpm/4o6Zv+KOmb/ijpm/4o6Zv9rJGt1ayRrNGska3W4S4f/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIo6Zv+KOmb/ijpm/4o6Zv+KOmb/ijpm/4o6Zv+KOmb/uEuHmQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC4S4d3uEuH/4o6Zv+KOmaHxszMN4o6ZoeKOmb/uEuH/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALhLh/8AAAAAAAAAAAAAAAAAAAAAuEuH/wAAAAAAAAAAAAAAAAAAAAAAAAAA+B8AAMAfAACYPwAAuB8AALgfAACcHwAAzD8AAPgfAADgAwAA5gMAAOZjAADnYwAA4HcAAPAHAAD4jwAA+98AAA==";

http_response favicon_view::handle(http_request const &request)  try {

    http_response result;

    result.body_ = decode_base64(favicosrc);
    result.mime_type_ = "image/x-icon";
    result.code_ = 200;
    return result;

} catch (...) {
    return http_response{};
}
