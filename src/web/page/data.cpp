#include <sstream>
#include "web/view.hpp"
#include "data.hpp"
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "data/Container.hpp"
#include "systems/Timer.h"
#include "common.hpp"

namespace {

std::string ms_to_str(time_t t) {
    return seconds_to_human_readable_str(static_cast<double>(t) / 1000);
}

void writeTableRow(std::ostream &html, const char *caption, std::shared_ptr<BaseData> data) {
    html << "<tr>";
    html << "<th>" << caption << "</th>";

    switch (data->get_state()) {
    case BaseDataState::NOT_LOADED:
        html << "<td colspan=3>" << (data->get_base() ? "Unable to load new data" : "Not loaded") << "</td>";
        break;
    case BaseDataState::LOADING:
        html << "<td colspan=3>Loading started "
             << ms_to_str(data->last_loading_time()) << " ago</td>";
        break;
    case BaseDataState::LOADED:
        html << "<td>" << string_time(data->get_time_in_ms() / 1000) << "</td>";
        html << "<td>" << data->get_base()->dataSize() / 1024 << " Kb</td>";
        html << "<td>" << data->get_base()->size() << "</td>";
        break;
    default:
        throw std::logic_error{"unknown BaseDataState value"};
    }

    html << "<td>" << ms_to_str(data->last_loading_time()) << "</td>";
    html << "<td>" << ms_to_str(data->total_loading_time()) << "</td>";
    html << "</tr>\n";
}

} // namespace

http_response data_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            os(&sb);

    render_header(os,"Data");

    auto data = Container::instance();

    os << "<table border=1 width=100%>";
    os << "<tr><th></th><th>Updated at</th><th>Size</th><th>Rows</th><th>Last time</th><th>Total time</th></tr>";

    for (auto [name, tab] : data->all_data()) {
        writeTableRow(os, name, tab);
    }

    return resp;
}
