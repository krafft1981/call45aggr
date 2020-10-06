#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include "db/BDb.h"
#include "db/BDbResult.h"
#include "log/log_instance.hpp"
#include "log/log.hpp"
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"
#include "data/model/CallRecord.h"
#include "MergeOnline.h"

int MergeOnline::getWatcherSize() {

    return onlineMap_.size();
}

void MergeOnline::statusHtml(std::ostream& out) {

    for(int id = 0; id < MergeOnline::getWatcherSize(); ++id) {
        out << "</br></br>";
        auto online = getData(id);
        out << "<table border='1'><caption>Online Link. server: " << id << " (" << online.size() << ")rows " << "</caption><tr><th>uniqueid_orig</th><th>uniqueid_term</th><th>start_time</th></tr>";
        for(auto link : online) {
            out << "<tr>";
            out << concat("<th>", link.uniqueid_orig,	"</th>");
            out << concat("<th>", link.uniqueid_term,	"</th>");
            out << concat("<th>", link.link_start_time,	"</th>");
            out << "</tr>";
        }

        out << "</table>";
    }
}

std::vector<CallLegLinkOnline> MergeOnline::getData(int id) {

    auto watcher = onlineMap_.find(id);
    if (watcher == onlineMap_.end()) {
        std::vector<CallLegLinkOnline> empty;
        return empty;
    }

    return watcher->second.getData();
}

MergeOnline::MergeOnline() {

    for (int id = 0; id < app::online_pool_count(); ++id) {
        onlineMap_.insert(make_pair(id, std::move(OnlineWatcher(id))));
        onlineMap_.find(id)->second.start();
    }
}

bool MergeOnline::checkUniqueIdOrig(std::string uniqueId) {

    if (!uniqueId.size())
        return false;

    for(auto it = onlineMap_.begin(); it != onlineMap_.end(); ++it) {
        if (it->second.checkUniqueIdOrig(uniqueId))
            return true;
    }

    return false;
}

bool MergeOnline::checkUniqueIdTerm(std::string uniqueId) {

    if (!uniqueId.size())
        return false;

    for(auto it = onlineMap_.begin(); it != onlineMap_.end(); ++it) {
        if (it->second.checkUniqueIdTerm(uniqueId))
            return true;
    }

    return false;
}

std::vector<CallLegLinkOnline> MergeOnline::OnlineWatcher::getData() {

    const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
    return data_;
}

bool MergeOnline::OnlineWatcher::checkUniqueIdOrig(std::string uniqueId) {

    while(!ready_)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bool result = false;

    if (!uniqueId.empty()) {
        const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
        for(auto row : data_) {
            if (!row.uniqueid_orig.empty() && (row.uniqueid_orig == uniqueId)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

bool MergeOnline::OnlineWatcher::checkUniqueIdTerm(std::string uniqueId) {

    while(!ready_)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bool result = false;

    if (!uniqueId.empty()) {
        const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
        for(auto row : data_) {
            if (!row.uniqueid_term.empty() && (row.uniqueid_term == uniqueId)) {
                result = true;
                break;
            }
        }
    }

    return result;
}

std::string MergeOnline::OnlineWatcher::formatDateTime(std::string date) {

        std::string out = date.substr(0, 22);
        out[10] = ' ';
        return out;
}

void MergeOnline::OnlineWatcher::run() {

    while(app::running() && (getStatus() == RUNNING)) {

        auto online = app::online_conn(id_);

        int sock = PQsocket(online->getConn());
        int waited = get_settings().table_reload_time();

        online->exec("BEGIN");

        BDbResult res = online->query("select * from call_leg_link_online");

        {
            const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
            this->data_.clear();
            while(res.next()) {
                CallLegLinkOnline row;
                row.uniqueid_orig   = res.get_s("uniqueid_orig"  );
                row.uniqueid_term   = res.get_s("uniqueid_term"  );
                row.link_start_time = res.get_s("link_start_time");
                data_.push_back(row);
            }

            ready_ = true;
        }

        online->exec("LISTEN \"monitor.call_link_online.insert\"");
        online->exec("LISTEN \"monitor.call_link_online.update\"");
        online->exec("LISTEN \"monitor.call_link_online.delete\"");
        online->exec("COMMIT");

        PGconn * conn = online->getConn();

        while (waited-- && app::running() && (getStatus() == RUNNING)) {

            fd_set mask;
            FD_ZERO(&mask);
            FD_SET(sock, &mask);

            struct timeval timer = {.tv_sec = 1, .tv_usec = 0 };

            if (select(sock + 1, &mask, NULL, NULL, &timer) < 0)
                break;

            PQconsumeInput(conn);
            PGnotify * notify;
            while ((notify = PQnotifies(conn)) != NULL) {
                Json::CharReaderBuilder builder;
                Json::Value root;
                Json::Value link;
                string errors;

                try {
                    {
                        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                        bool parsingSuccessful = reader->parse(notify->extra, notify->extra + strlen(notify->extra), &root, &errors);
                    }

                    link = root["link"];

                    CallLegLinkOnline row;
                    row.uniqueid_orig   = link["uniqueid_orig"].asString();
                    row.uniqueid_term   = link["uniqueid_term"].asString();
                    row.link_start_time = formatDateTime(link["link_start_time"].asString());

                    const std::lock_guard<Spinlock> lockGuard{ this->lock_ };

                    string event = notify->relname;
                    if (event == "monitor.call_link_online.insert") {
                        row.channame_orig     = link["channame_orig"].asString();
                        row.channame_term     = link["channame_term"].asString();
                        row.link_start_time   = formatDateTime(link["link_start_time"].asString());
                        data_.push_back(row);
                    }

                    if (event == "monitor.call_link_online.update") {
                        for (auto it = data_.begin(); it != data_.end(); ++it) { 
                            if ((it->uniqueid_orig == row.uniqueid_orig) && (it->uniqueid_term == row.uniqueid_term)) {
                                it->channame_orig     = link["channame_orig"].asString();
                                it->channame_term     = link["channame_term"].asString();
                                it->link_start_time   = formatDateTime(link["link_start_time"].asString());
                                break;
                            }
                        }
                    }

                    if (event == "monitor.call_link_online.delete") {
                        for (auto it = data_.begin(); it != data_.end(); ++it) { 
                            if ((it->uniqueid_orig == row.uniqueid_orig) && (it->uniqueid_term == row.uniqueid_term)) {
                                it = data_.erase(it);
                                break;
                            }
                        }
                    }
                }

                catch(exception e) {
                    Log::warning(e.what());
                }

                PQfreemem(notify);
            }
        }
    }
}
