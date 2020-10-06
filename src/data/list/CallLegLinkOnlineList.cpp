#include "CallLegLinkOnlineList.h"

std::string CallLegLinkOnlineList::sql(BDb* db) {
    (void)db; // TODO: FIX UNUSED
    return "select * from call_leg_link_online";
}

void CallLegLinkOnlineList::parse_item(BDbResult& row, CallLegLinkOnline* item) const {

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

void CallLegLinkOnlineList::after_load() {

    for (CallLegLinkOnline &cll : data) {
        uniqueid_.insert(cll.uniqueid_orig);
        uniqueid_.insert(cll.uniqueid_term);

        uniqueidOrig_[cll.uniqueid_orig] = cll;
        uniqueidTerm_[cll.uniqueid_term] = cll;
    }
}

void CallLegLinkOnlineList::before_load() {

    uniqueid_.clear();
}

bool CallLegLinkOnlineList::uniqueid_check(std::string uniqueid) {

        return uniqueid_.find(uniqueid) != uniqueid_.end();
}

CallLegLinkOnline* CallLegLinkOnlineList::get_orig_term(std::string uniqueid_orig, std::string uniqueid_term) {

    
}
