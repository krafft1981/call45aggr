#include "CallLegLinkList.h"

CallLegLinkList::CallLegLinkList(InputTuple const &tup)
    : startTime_(std::get<0>(tup)),
      stopTime_ (std::get<1>(tup)),
      lastTime_ (startTime_)
{}

std::string CallLegLinkList::sql(BDb* db) {
    (void)db; // TODO: FIX UNUSED

    return concat("select * from call_leg_link where ",
        "link_start_time > '", startTime_, "' and ",
        "link_start_time <= '", stopTime_, "'"
    );
}

std::string const CallLegLinkList::getLastTime() const {
    return lastTime_;
}

void CallLegLinkList::parse_item(BDbResult& row, CallLegLink* item) const {

    item->id                = row.get_ll("call_leg_link_id");
    item->server_id         = row.get_i("server_id");
    item->uniqueid_orig     = row.get_s("uniqueid_orig");
    item->channame_orig     = row.get_s("channame_orig");
    item->uniqueid_term     = row.get_s("uniqueid_term");
    item->channame_term     = row.get_s("channame_term");
    item->link_start_time   = row.get_s("link_start_time");
    item->link_answer_time  = row.get_s("link_answer_time");
    item->link_end_time     = row.get_s("link_end_time");
    item->rec_filename      = row.get_s("rec_filename");
    item->status            = row.get_s("status");
    item->bridged           = row.get_b("bridged");
    item->cf                = row.get_s("cf");
    item->trf_by            = row.get_s("trf_by");
    item->queue_callid      = row.get_s("queue_callid");
    item->object_type       = row.get_s("object_type");
    item->object_data       = row.get_s("object_data");
}

void CallLegLinkList::after_load() {

    lastTime_ = data.front().link_start_time;

    for (CallLegLink &cll : data) {

        if (parseDateTime(lastTime_.c_str()) < parseDateTime(cll.link_start_time.c_str())) {
            lastTime_ = cll.link_start_time;
        }

        auto orig = origRecords_.find(cll.uniqueid_orig);
        if (orig != origRecords_.end()) {
            origRecords_[cll.uniqueid_orig].push_back(&cll);
            continue;
        }

        std::vector<CallLegLink*> record;
        record.push_back(&cll);
        origRecords_[cll.uniqueid_orig] = record;
    }
}

std::vector<CallLegLink*> CallLegLinkList::getByUniqueIdOrig(std::string uniqueid) const {

    auto it = origRecords_.find(uniqueid);
    if (it == origRecords_.end()) {
        std::vector<CallLegLink*> empty;
        return empty;
    }

    return it->second;
}
