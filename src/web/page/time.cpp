//
// Created by root on 11/12/19.
//

#include "web/view.hpp"
#include "time.hpp"
#include "rendering.hpp"
#include "data/Repository.hpp"
#include "daemon/daemon.hpp"
#include "common/string_streambuf.hpp"
#include "common.hpp"

http_response time_view::handle(const http_request &request) {

    http_response resp;
    resp.code_ = 200;
    resp.mime_type_ = "text/html";

    utils::string_streambuf sb(resp.body_);
    std::ostream html(&sb);

    render_header(html, "time");

    html << "<table border=1><tr><th>start</th><th>" << string_time(app::getMergeService()->getData()->getStart()) << "</th></tr>";
    html << "<tr><th>leg time</th><th>" << string_time(app::getMergeService()->getData()->getMaxLegTime()) << "</th></tr>";
    html << "<tr><th>link time</th><th>" << string_time(app::getMergeService()->getData()->getMaxLinkTime()) << "</th></tr>";
    html << "<tr><th>processed second</th><th>" << app::getMergeService()->getData()->getMaxLegTime() - app::getMergeService()->getData()->getStart() << "</th></tr>";
    html << "</table>";

    return resp;
}
