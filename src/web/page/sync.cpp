
#include <web/view.hpp>
#include "sync.hpp"
#include "rendering.hpp"
#include <common/string_streambuf.hpp>


#include <cstdio>

#include <data/sync/Puller.h>
#include <data/sync/PullManager.h>
#include <common/string_streambuf.hpp>

namespace {

    std::string get_speed_mbps(const Timer& timer, std::size_t n_bytes, double seconds) {
        if (timer.nDoneMeasurements() == 0) {
            return "?";
        }
        const double speed_mbps = static_cast<double>(n_bytes) / seconds / (1024.0 * 1024.0 / 8.0);
        char speed_mbps_buf[128];
        std::snprintf(speed_mbps_buf, sizeof(speed_mbps_buf), "%.3f", speed_mbps);
        return { speed_mbps_buf };
    }

} // namespace

struct kb_printer {
    size_t value_;

    explicit kb_printer(size_t value)
            : value_(value) {}

    friend std::ostream& operator << (std::ostream& os, kb_printer const& val) {
        os << (val.value_/1024)  << "k";
        return os;
    }
};

http_response sync_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            os(&sb);

    render_header(os,"Sync");

    auto& html = os;

        auto manager = PullManager::instance();

    //html << "Mode: <b>" << app().conf.getBillerModeName() << "</b>&nbsp; \n";
    //html << "Region: <b>" << app().conf.get_sql_regions_list() << "</b>&nbsp; \n";
    html << "Errors count: <b>" << manager->get_errors_count() << "</b>\n";
    html << "<br/><br/>\n";

    html << "<table width=100% border=1>\n";
    html << "<tr>";
    html << "<th>&nbsp;</th>";
    html << "<th>Full</th>";
    html << "<th>Partial</th>";
    html << "<th>Errors</th>";
    html << "<th>Current status</th>";
    html << "<th>Last time</th>";
    html << "<th>Last bytes</th>";
    html << "<th>Last speed, Mbps</th>";
    html << "<th>Thread</th>";
    html << "<th>Total time</th>";
    html << "<th>Total bytes</th>";
    html << "<th>Average speed, Mbps</th>";
    html << "<tr>\n";
    for (const auto pull: manager->get_pulls()) {
        const auto info = pull->get_info();
        const size_t last_copied = pull->copied_bytes_in_last_operation();
        html << "<tr>";
        html << "<th>" << pull->get_event() << "</th>";
        html << "<td>" << to_string(info.pull_count_full) << "</td>";
        html << "<td>" << to_string(info.pull_count_partial) << "</td>";
        html << "<td>" << to_string(info.pull_count_errors) << "</td>";
        html << "<td>" << info.timer.currentStatus() << "</td>";
        html << "<td>" << info.timer.lastMeasuredIntervalStr() << "</td>";
        if (info.timer.nDoneMeasurements() == 0) {
            if (last_copied == 0) {
                html << "<td>?</td>";
            } else {
                html << "<td>" << kb_printer(last_copied) << "</td>";
            }
        } else if (last_copied != info.last_copied_bytes) {
            html << "<td>" << kb_printer(last_copied) << "/" << kb_printer(info.last_copied_bytes) << "</td>";
        } else {
            html << "<td>" << kb_printer(info.last_copied_bytes) << "</td>";
        }
        html << "<td>" << get_speed_mbps(info.timer, info.last_copied_bytes, info.timer.lastMeasuredInterval()) << "</td>";
        html << "<td>" << pull->priority() << "</td>";
        html << "<td>" << info.timer.totalMeasuredIntervalsStr() << "</td>";
        html << "<td>" << kb_printer(info.total_copied_bytes) << "</td>";
        html << "<td>" << get_speed_mbps(info.timer, info.total_copied_bytes, info.timer.totalMeasuredIntervals()) << "</td>";
        html << "</tr>\n";
    }
    html << "</table><br/>\n";
    return resp;
}
