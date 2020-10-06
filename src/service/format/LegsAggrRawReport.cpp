#include <iostream>
#include "data/model/LegsAggrRaw.h"
#include "daemon/settings.hpp"
#include "LegsAggrRawReport.h"

LegsAggrRawReport::LegsAggrRawReport() : newCharReader(builder.newCharReader())
{}

std::string LegsAggrRawReport::header() {

    return "INSERT INTO legs_aggr_raw (start_time, stop_time, uniqueid, object_type, object_type_l, sip_ip, sip_ip_l, status, call_id_md5, call_id_md5_l, to_jsonb) values ";
}

void LegsAggrRawReport::build(CallRecord * record, std::ostream& out) {

    LegsAggrRawRecord r;

    r.start_time       = record->a.start_time.empty() ? record->b.front().start_time : record->a.start_time;
    r.stop_time        = record->b.front().stop_time.empty() ? record->a.stop_time   : record->b.front().stop_time;
    r.uniqueid         = record->a.uniqueid;
    r.object_type      = record->a.object_type;
    r.object_type_l    = record->b.front().object_type;
    r.sip_ip           = record->a.sip_ip;
    r.sip_ip_l         = record->b.front().sip_ip;
    r.status           = "ANSWERED";
    r.call_id_md5      = record->a.call_id_md5;
    r.call_id_md5_l    = record->b.front().call_id_md5;
    r.to_jsonb["orig"] = record->a.json();
    r.to_jsonb["last"] = record->b.front().json();

    r.dump(out);
}
