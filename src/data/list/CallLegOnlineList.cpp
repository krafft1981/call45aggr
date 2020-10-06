#include "CallLegOnlineList.h"

std::string CallLegOnlineList::sql(BDb* db) {
    (void)db; // TODO: FIX UNUSED
    return "select * from call_leg_online";
}

void CallLegOnlineList::parse_item(BDbResult& row, CallLegOnline* item) const {

    item->uniqueid         = row.get_s("uniqueid");
    item->channame         = row.get_s("channame");
    item->orig             = row.get_b("orig");
    item->account_id       = row.get_i("account_id");
    item->vpbx_id          = row.get_i("vpbx_id");
    item->start_time       = row.get_s("start_time");
    item->answer_time      = row.get_s("answer_time");
    item->stop_time        = row.get_s("stop_time");
    item->final_status     = row.get_s("final_status");
    item->q931_status      = row.get_i("q931_status");
    item->hangup_cause     = row.get_s("hangup_cause");
    item->number           = row.get_s("number");
    item->name             = row.get_s("name");
    item->did              = row.get_ll("did");
    item->did_type         = row.get_s("did_type");
    item->object_type      = row.get_s("object_type");
    item->object_id        = row.get_i("object_id");
    item->sip_ip           = row.get_s("sip_ip");
    item->last_linked_id   = row.get_s("last_linked_id");
    item->sip_call_hash_id = row.get_s("sip_call_hash_id");
    item->extra_info_id    = row.get_s("extra_info_id");
}

void CallLegOnlineList::after_load() {

    for (CallLegOnline &clo : data) {
        uniqueid_.insert(clo.uniqueid);
    }
}

bool CallLegOnlineList::uniqueid_check(std::string uniqueid){

        return uniqueid_.find(uniqueid) != uniqueid_.end();
}
