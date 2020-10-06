#include <iomanip>
#include "state.hpp"
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "web/view.hpp"
#include "common.hpp"
#include "daemon/daemon.hpp"
#include "common/split.h"
#include "merge.hpp"
#include "daemon/health_register.hpp"

http_response merge_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            html(&sb);

    render_header(html, "Merge");
    html << "<table border=1>";
    html << concat("<tr><th>OutQueue</th><th>", app::getMergeService()->getOutputQueueSize(), "</th></tr>");
    html << concat("<tr><th>unowned clt</th><th>", app::getMergeService()->getData()->getLegSize(), "</th></tr>");
    std::tuple<int, int> size = app::getMergeService()->getData()->getLinkSize();
    html << concat("<tr><th>unowned cll</th><th>", std::get<0>(size), " / ", std::get<1>(size), "</th></tr>");
    html << concat("<tr><th>clo</th><th>", app::getMergeService()->getData()->getDataSize(), "</th></tr>");
    html << "</table>";
    return resp;
}
