#include <iostream>
#include <time.h>
#include "db/BDbResult.h"
#include "db/BDb.h"
#include "log/log_instance.hpp"
#include "log/log.hpp"
#include "daemon/settings.hpp"
#include "daemon/daemon.hpp"
#include "ReadCallLeg.h"

void ReadCallLeg::statusHtml(std::ostream& out) {

    out << "<table border=1>";
    out << concat("<tr><th>paused</th><th>", paused_ ? "Yes" : "No", "</th></tr>");
    out << concat("<tr><th>startTime</th><th>", string_time(startTime_), "</th></tr>");
    out << concat("<tr><th>stopTime</th><th>", string_time(stopTime_), "</th></tr>");
    out << concat("<tr><th>queue size</th><th>", output_->size_approx(), "</th></tr>");
    out << "</table>";
}

void ReadCallLeg::pause() {

    paused_ = true;
}

void ReadCallLeg::resume() {

    paused_ = false;
}

bool ReadCallLeg::getPaused() {

    return paused_;
}

std::time_t ReadCallLeg::getTime() {

    return startTime_;
}

void ReadCallLeg::setStart(std::time_t stemp) {

    startTime_ = stemp;
    stopTime_  = stemp + get_settings().read_threshold_second();
}

void ReadCallLeg::run() {

    while (app::running() && (getStatus() == RUNNING)) {

        if (stopTime_ == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        while(stopTime_ > (time(NULL) - get_settings().read_threshold_second()))
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        while (!app::source_conn()->query(concat("select 1 from call_leg where start_time > TO_TIMESTAMP(", stopTime_, ") and start_time <= TO_TIMESTAMP(", time(NULL) - get_settings().read_waiting_second(), ") limit 1")).size())
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        int count = getSQLData(startTime_, stopTime_);

        Log::notice(concat("sql leg: ", startTime_, " -> ", stopTime_, " returned: ", count, " records"));

        startTime_ = stopTime_;

        std::string sql = concat("select 1 from call_leg where start_time > TO_TIMESTAMP(", stopTime_ + get_settings().max_call_time(), ") and start_time <= TO_TIMESTAMP(", time(NULL) - get_settings().read_waiting_second(), ") limit 1");
        if (app::source_conn()->query(sql).size())
                stopTime_ += get_settings().max_call_time();
        else    stopTime_ += get_settings().read_threshold_second();
    }
}

int ReadCallLeg::getSQLData(std::time_t start, std::time_t stop) {

    while (paused_)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::string sql = concat("select * from call_leg where start_time > TO_TIMESTAMP(", start, ") and start_time <= TO_TIMESTAMP(", stop, ") order by call_leg_id");
    auto row = app::source_conn()->query(sql);

    while(row.next()) {
        CallLeg leg;
        leg.id               = row.get_ll("call_leg_id");
        leg.server_id        = row.get_i("server_id");
        leg.uniqueid         = row.get_s("uniqueid");
        leg.channame         = row.get_s("channame");
        leg.orig             = row.get_b("orig");
        leg.account_id       = row.get_i("account_id");
        leg.vpbx_id          = row.get_i("vpbx_id");
        leg.start_time       = row.get_s("start_time");
        leg.answer_time      = row.get_s("answer_time");
        leg.stop_time        = row.get_s("stop_time");
        leg.final_status     = row.get_s("final_status");
        leg.q931_status      = row.get_i("q931_status");
        leg.hangup_cause     = row.get_s("hangup_cause");
        leg.number           = row.get_s("number");
        leg.name             = row.get_s("name");
        leg.did              = row.get_ll("did");
        leg.did_type         = row.get_s("did_type");
        leg.object_type      = row.get_s("object_type");
        leg.object_id        = row.get_i("object_id");
        leg.sip_ip           = row.get_s("sip_ip");
        leg.last_linked_id   = row.get_s("last_linked_id");
        leg.sip_call_hash_id = row.get_s("sip_call_hash_id");
        leg.call_id_md5      = row.get_s("call_id_md5");
        leg.extra_info_id    = row.get_s("extra_info_id");

//        if (readId_[leg.server_id]) {
//            if (abs(readId_[leg.server_id] - leg.id) != 1) {
//                std::cerr << "server: " << leg.server_id << " razriv: " << abs(readId_[leg.server_id] - leg.id) << " last value: " << readId_[leg.server_id] << " new value: " << leg.id << " uniqueid: " << leg.uniqueid << std::endl;
//            }
//        }

        readId_[leg.server_id] = leg.id;

        while (output_->size_approx() >= get_settings().max_queue_size())
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        output_->enqueue(leg);
    }

    return row.size();
}
