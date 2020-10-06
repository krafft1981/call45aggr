#include <iomanip>
#include "state.hpp"
#include "rendering.hpp"
#include "common/string_streambuf.hpp"
#include "web/view.hpp"
#include "daemon/application.hpp"
#include "common/split.h"
#include "daemon/health_register.hpp"

http_response state_view::handle(const http_request &request) {
    http_response resp;

    resp.mime_type_ = "text/html";
    utils::string_streambuf sb(resp.body_);
    std::ostream            os(&sb);

    // Пересылка реакции на нажатие происходит через POST. Скидываем ошибки
    for (auto& s : split_any(request.body_.to_string(), "&")){
        if (s == "action=do_reset") {
            health_register::clear();
        }
    }

    render_header(os, "State");

    // os <<  "<pre>Not implemented</pre>";

    os << "</br></br>";

    os << R"(
<table border="1">
 <caption>Periodic tasks</caption>
 <tr>
    <th>Name</th>
    <th>Timeout (sec)</th>
    <th>Last duration(ms)</th>
    <th>Total duration(s)</th>
    <th>Count</th>
    <th>Error</th>
 <tr>
)";

    os << std::fixed;
    os.precision(2);

    time_t cur_time = time(nullptr);
    for (auto& [name, timeout, last_duration, total_duration, start_ts, runs_count, errors_count] : application::instance().cyclic_tasks()) {
        os << "<tr>";
        os << "<td>" << " <a href='/cyclic_task?name=" << name << "'>" << name << "</a>" << "</td>";

        if (timeout % 1000 == 0) {
            os << "<td>" << timeout / 1000 << "</td>";
        } else {
            os << "<td>" << static_cast<double>(timeout)/1000 << "</td>";
        }

        if (start_ts != 0) {
            time_t runtime = cur_time - start_ts;

            time_t hours = runtime/3600; runtime = runtime % 3600;
            time_t minutes = runtime / 60;
            time_t seconds = runtime % 60;

            os << "<td>running ";
            if (hours != 0) {
                os << std::setw(2) << std::setfill('0') << hours << ":";
            }

            if (minutes != 0) {
                os << std::setw(2) << std::setfill('0') << minutes << ":";
            }

            os << std::setw(2) << std::setfill('0') << seconds << "s";
            os << "</td>";

        } else {
            os << "<td>" << static_cast<double>(last_duration)/1000 << "</td>";
        }
        os << "<td>" << static_cast<double>(total_duration)/1000000 << "</td>";
        os << "<td>" << runs_count << "</td>";
        os << "<td>" << errors_count << "</td>";
        os << "</tr>";
    }
    os << "</table><br/>";

    os << R"(<form action="" method="POST"> <button name="action" value="do_reset">Reset errors</button></form>)";

    return resp;
}
