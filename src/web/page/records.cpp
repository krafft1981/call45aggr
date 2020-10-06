#include <iostream>
#include <iomanip>
#include <json/json.h>
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "web/view.hpp"
#include "common.hpp"
#include "db/BDbResult.h"
#include "db/BDb.h"
#include "daemon/daemon.hpp"
#include "common/split.h"
#include "records.hpp"
#include "daemon/health_register.hpp"

http_response records_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            html(&sb);

    render_header(html, "Link");

    html << "<table border=1>";
    html << "<tr>";
    html << "<th>start_time</th>";
    html << "<th>stop_time</th>";
    html << "<th>uniqueid</th>";
    html << "<th>object_type</th>";
    html << "<th>object_type_l</th>";
    html << "<th>sip_ip</th>";
    html << "<th>sip_ip_l</th>";
    html << "<th>status</th>";
    html << "<th>call_id_md5</th>";
    html << "<th>call_id_md5_l</th>";
    html << "<th>to_jsonb</th>";
    html << "</tr>";

    auto res = app::output_conn()->query("select * from legs_aggr_raw where start_time > (select max(start_time) from legs_aggr_raw) - interval '30 sec' order by start_time");
    while (res.next()) {
        html << "<tr>";
        html << concat("<th>", res.get_s("start_time"), "</th>");
        html << concat("<th>", res.get_s("stop_time"), "</th>");
        html << concat("<th>", res.get_s("uniqueid"), "</th>");
        html << concat("<th>", res.get_s("object_type"), "</th>");
        html << concat("<th>", res.get_s("object_type_l"), "</th>");
        html << concat("<th>", res.get_s("sip_ip"), "</th>");
        html << concat("<th>", res.get_s("sip_ip_l"), "</th>");
        html << concat("<th>", res.get_s("status"), "</th>");
        html << concat("<th>", res.get_s("call_id_md5"), "</th>");
        html << concat("<th>", res.get_s("call_id_md5_l"), "</th>");
        html << concat("<th>", res.get_s("to_jsonb"), "</th>");
        html << "</tr>";
    }
    html << "</table>";
    return resp;
}
