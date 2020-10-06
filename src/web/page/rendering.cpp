
#include <unordered_map>
#include <vector>
#include <iomanip>
#include "rendering.hpp"
#include "common.hpp"
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"

// Название, путь без слеша
static std::vector<std::pair<std::string, std::string>> header {
        {"Data", "data"},
        {"Online", "online"},
        {"Time", "time"},
        {"Merge", "merge"},
        {"leg", "leg"},
        {"link", "link"},
        {"Sync", "sync"},
        {"State", "state"},
        {"Records", "records"},
        {"Config", "config"},
        {"Health", "health"},
        {"Log", "log"}
};

void render_header(std::ostream& html, char const* active) {

    html << R"(<head><meta charset="utf-8"></head>)";

    html << "<style>\n";
    html << "* {font-family:monospace !important;}\n";
    html << "td { text-align: center }\n";
    html << "mark { \n"
            "    background-color: grey;\n"
                 "    color: black;\n"
                 "}\n";
    html << "</style>\n";

    auto settings = get_settings();

    for (auto h : header) {
        // Также пофильтровать по какому-нибудь полю
        // if (!h.thread.empty() && !app().threads.isRegistered(h.thread))
        //    continue;

        html << " <a href='/"+h.second+"'>";

         if (h.first == active) {
            html << "<mark>" << h.first << "</mark>";
         } else {
            html << h.first;
         }

         html <<"</a> | ";
    }
    // html << " <a href='/health?cmd=view&view=AutoTest'>AutoTest</a> |";

    html << string_time(time(NULL)) << " ";

    time_t uptm = app::uptime();
    time_t hours = uptm/3600; uptm = uptm % 3600;
    time_t minutes = uptm/60;
    time_t seconds = uptm%60;

    html << "<b>UPTIME: " << std::setw(2) << std::setfill('0') << hours << ":" <<
                             std::setw(2) << std::setfill('0') << minutes << ":" <<
                             std::setw(2) << std::setfill('0')<< seconds << "</b>";

    //AppStatus status = app().getStatus();
    //AppStatus real_status = app().getRealStatus();
    //html << "<b>" << AppStatusNames[real_status] << "</b>";
    //if (status != real_status)
    //   html << " (" << AppStatusNames[status] << ")";

    html << "\n<hr/><br/>\n";
}