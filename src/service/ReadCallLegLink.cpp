#include <iostream>
#include <time.h>
#include <json/json.h>
#include "db/BDbResult.h"
#include "db/BDb.h"
#include "log/log_instance.hpp"
#include "log/log.hpp"
#include "daemon/settings.hpp"
#include "daemon/daemon.hpp"
#include "ReadCallLegLink.h"

void ReadCallLegLink::statusHtml(std::ostream& out) {

    out << "<table border=1>";
    out << concat("<tr><th>paused</th><th>", paused_ ? "Yes" : "No", "</th></tr>");
    out << concat("<tr><th>startTime</th><th>", string_time(startTime_), "</th></tr>");
    out << concat("<tr><th>stopTime</th><th>", string_time(stopTime_), "</th></tr>");
    out << concat("<tr><th>queue size</th><th>", output_->size_approx(), "</th></tr>");

    const std::lock_guard<Spinlock> lockGuard { this->lock_ };

    if (waitLinkOrig_.size())
        out << concat("<tr><th>wait uniqueId orig</th><th>", waitLinkOrig_, "</th></tr>");

    if (waitLinkTerm_.size())
        out << concat("<tr><th>wait uniqueId term</th><th>", waitLinkTerm_, "</th></tr>");

    out << "</table>";
}

void ReadCallLegLink::pause() {

    paused_ = true;
}

void ReadCallLegLink::resume() {

    paused_ = false;
}

bool ReadCallLegLink::getPaused() {

    return paused_;
}

std::time_t ReadCallLegLink::getTime() {

    return startTime_;
}

MergeOnline* ReadCallLegLink::getOnline() {

    return &online_;
}

void ReadCallLegLink::setStart(std::time_t stemp) {

    startTime_ = stemp;
    stopTime_  = stemp + get_settings().read_threshold_second();
}

void ReadCallLegLink::run() {

    while (app::running() && (getStatus() == RUNNING)) {

        if (stopTime_ == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        while(stopTime_ > (time(NULL) - get_settings().read_threshold_second()))
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        while (!app::source_conn()->query(concat("select 1 from call_leg_link where link_start_time > TO_TIMESTAMP(", stopTime_, ") and link_start_time <= TO_TIMESTAMP(", time(NULL) - get_settings().read_waiting_second(), ") limit 1")).size())
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        int count = getSQLData(startTime_, stopTime_);

        Log::notice(concat("sql link: ", startTime_, " -> ", stopTime_, " returned: ", count, " records"));

        startTime_ = stopTime_;

        std::string check = concat("select 1 from call_leg_link where link_start_time > TO_TIMESTAMP(", stopTime_ + get_settings().max_call_time(), ") and link_start_time <= TO_TIMESTAMP(",  time(NULL) - get_settings().read_waiting_second(), ") limit 1");
        if (app::source_conn()->query(check).size())
                stopTime_ += get_settings().max_call_time();
        else    stopTime_ += get_settings().read_threshold_second();
    }
}

int ReadCallLegLink::getSQLData(std::time_t start, std::time_t stop) {

    while (paused_)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::string sql = concat("select * from call_leg_link where link_start_time > TO_TIMESTAMP(", start, ") and link_start_time <= TO_TIMESTAMP(", stop, ") order by call_leg_link_id, server_id");
    auto row = app::source_conn()->query(sql);
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> newCharReader(builder.newCharReader());
    while(row.next()) {
        CallLegLink link;
        link.id                 = row.get_ll("call_leg_link_id");
        link.server_id          = row.get_i("server_id");
        link.uniqueid_orig      = row.get_s("uniqueid_orig");
        link.channame_orig      = row.get_s("channame_orig");
        link.uniqueid_term      = row.get_s("uniqueid_term");
        link.channame_term      = row.get_s("channame_term");
        link.link_start_time    = row.get_s("link_start_time");
        link.link_answer_time   = row.get_s("link_answer_time");
        link.link_end_time      = row.get_s("link_end_time");
        link.rec_filename       = row.get_s("rec_filename");
        link.status             = row.get_s("status");
        link.bridged            = row.get_b("bridged");
        link.cf                 = row.get_s("cf");
        link.trf_by             = row.get_s("trf_by");
        link.queue_callid       = row.get_s("queue_callid");
        link.object_type        = row.get_s("object_type");

        std::string object_data = row.get_s("object_data");
        std::string error;
        newCharReader->parse(&object_data[0], &object_data[object_data.size()], &link.object_data, &error);

        while (online_.checkUniqueIdOrig(link.uniqueid_orig)) {
            {
                const std::lock_guard<Spinlock> lockGuard { this->lock_ };
                waitLinkOrig_= link.uniqueid_orig;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        {
            const std::lock_guard<Spinlock> lockGuard { this->lock_ };
            waitLinkOrig_ = "";
        }

        while (online_.checkUniqueIdTerm(link.uniqueid_term)) {
            {
                const std::lock_guard<Spinlock> lockGuard { this->lock_ };
                waitLinkTerm_= link.uniqueid_term;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        {
            const std::lock_guard<Spinlock> lockGuard { this->lock_ };
            waitLinkTerm_ = "";
        }

        while (output_->size_approx() >= get_settings().max_queue_size())
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        output_->enqueue(link);
    }

    return row.size();
}
