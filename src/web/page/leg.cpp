#include <iomanip>
#include "state.hpp"
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "web/view.hpp"
#include "common.hpp"
#include "daemon/daemon.hpp"
#include "common/split.h"
#include "leg.hpp"
#include "daemon/health_register.hpp"

http_response leg_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            html(&sb);

    render_header(html, "Leg");
    app::getMergeService()->getData()->legStatusHtml(html);
    return resp;
}
