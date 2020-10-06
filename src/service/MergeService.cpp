#include <iostream>
#include <stdlib.h>
#include <sstream>
#include "db/BDbResult.h"
#include "log/log.hpp"
#include "log/log_instance.hpp"
#include "db/BDb.h"
#include "MergeService.h"
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"
#include "service/format/DataVoipRecordReport.h"
#include "service/format/PstnRecordReport.h"
#include "service/format/LegsAggrRawReport.h"

int MergeService::getOutputQueueSize() {

    return output_.size_approx();
}

MergeContainer* MergeService::getData() {

    return &data_;
}

class LegCompareByStartTime {
public:
    bool operator() (const CallLeg &a, const CallLeg &b) {
        return a.start_time < b.start_time;
    }
} legCompareByStartTime;

class LinkCompareByLinkId {
public:
    bool operator() (const CallLegLink &a, const CallLegLink &b) {
        return a.id < b.id;
    }
} linkCompareByLinkId;

void MergeService::run() {

    std::time_t stemp = 0;

    {
        std::string start_time;
        auto res = app::output_conn()->query("select begin_connection_time as start_time from datavoiprecord where begin_connection_time = (select max(begin_connection_time) from datavoiprecord)");
        if (res.size()) {
            while(res.next())
                start_time = res.get_s("start_time");
        }
        else {
            auto res = app::source_conn()->query("select min(start_time) as start_time from call_leg");
            while(res.next())
                start_time = res.get_s("start_time");
        }

        if (start_time.size())
            stemp = parseDateTime(start_time.c_str());
    }

    if (!stemp)
        stemp = time(NULL);

    Log::notice(concat("start from: ", stemp));

    data_.setStart(stemp);
    data_.start();

//    DataVoipRecordReport dvrr;
//    LegsAggrRawReport trr;

    while(app::running()) {

        std::vector<CallRecord> records;
        if (!output_.try_dequeue_bulk(std::back_inserter(records), get_settings().max_queue_size())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

//        std::string bufferDvrr(get_settings().sql_insert_buffer_size(), '\0');
//        std::string bufferTrr (get_settings().sql_insert_buffer_size(), '\0');

//        ostringstream ossDvrr(bufferDvrr);
//        ostringstream ossTrr (bufferTrr);

//        auto output = app::output_conn();
//        output->exec("BEGIN");

        int maxCallRecordTime = 0;

        for(auto record : records) {

            if (maxCallRecordTime < record.startTime)
                maxCallRecordTime = record.startTime;

            for(auto leg : record.legs) {
                if(leg.second.orig)
                    record.a = leg.second;

                if(!leg.second.last_linked_id.empty())
                    record.b.push_back(record.legs[leg.second.last_linked_id]);
            }

            std::sort(record.b.begin(),     record.b.end(),     legCompareByStartTime);
            std::sort(record.links.begin(), record.links.end(), linkCompareByLinkId  );
/*
            if (ossTrr .tellp() != 0) ossTrr  << ",";
//            if (ossDvrr.tellp() != 0) ossDvrr << ",";

            trr .build(&record, ossTrr );
//            dvrr.build(&record, ossDvrr);

            if (ossTrr.tellp() > (bufferTrr.length())) {
                output->exec(concat(trr.header(), ossTrr.str()));
                ossTrr.clear();
                ossTrr.str("");
            }

            if (ossDvrr.tellp() > (bufferDvrr.length())) {
                output->exec(concat(dvrr.header(), ossDvrr.str()));
                ossDvrr.clear();
                ossDvrr.str("");
            }
*/
        }

//        std::cerr << concat(trr.header() , ossTrr.str()) << std::endl;

//        output->exec(concat(trr.header() , ossTrr.str())) ;
//        output->exec(concat(dvrr.header(), ossDvrr.str()));
//        output->exec("COMMIT");

        Log::notice(concat("update transaction time: ", maxCallRecordTime, " with: ", records.size(), " records"));
    }

    data_.stop();
}
