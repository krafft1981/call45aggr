#include <json/json.h>
#include <algorithm>
#include <iostream>
#include "db/BDbResult.h"
#include "db/BDb.h"
#include "data/model/DataVoipRecord.h"
#include "daemon/settings.hpp"
#include "daemon/daemon.hpp"
#include "common.hpp"
#include "DataVoipRecordReport.h"

std::string DataVoipRecordReport::header() {

    return "INSERT INTO datavoiprecord (telco_id, begin_connection_time, end_connection_time, client_info, server_info, point_id, voip_session_id, voip_conference_id, voip_duration, voip_originator_name, voip_call_type_id, calling_original_number, calling_translated_number, calling_e164_number, called_original_number, called_translated_number, called_e164_number, voip_in_bytes_count, voip_out_bytes_count, voip_fax, voip_term_cause, inbound_trunk, outbound_trunk, voip_gateways, voip_protocol, supplement_service_id, voip_abonent_id, voip_nat_info, voip_location, voip_event, voip_data_content_id, vpbx_id, server_id, account_id, account_id_2, orig_uniqueid, raw_data) values ";
}

void DataVoipRecordReport::build(CallRecord * record, std::ostream& out) {

    DataVoipRecord r;
    Json::Value json;

    int supplement_service_id = 0;
    std::string raw_data = "";

//    std::string client_info;
//    std::string server_info;
//    std::string voip_session_id;
//    std::string voip_conference_id;
//    std::string voip_abonent_id;
//    std::string voip_nat_info;
//    std::string voip_location;

    if (record->a.object_type == "pstn")
        r.voip_event = "incoming";

    for(auto link : record->links) {
        if (link.uniqueid_orig == record->a.uniqueid) {
            if ((record->a.object_type != "pstn") && (record->a.object_type != "webcall") && (record->a.object_type != "missed_call_callback") && (record->legs[link.uniqueid_term].object_type != "pstn"))
                r.voip_event = "internal";

            if (record->legs[link.uniqueid_term].object_type == "pstn")
                r.voip_event = "outgoing";
        }
    }

    if (r.voip_event.empty())
        r.voip_event = "uncknown";

    r.begin_connection_time = record->a.start_time;
    r.end_connection_time = record->b.size() ? record->b.back().stop_time : record->a.stop_time;

    if (record->b.size())
            r.end_connection_time = record->b.back().stop_time;
    else    r.end_connection_time = record->a.stop_time;

    if (record->a.number.empty())
            r.calling_original_number = r.calling_translated_number = r.calling_e164_number = concat(record->a.did);
    else    r.calling_original_number = r.calling_translated_number = r.calling_e164_number = escape_string(record->a.number);

    std::string b;
    for(auto leg : record->b) {
        std::string number;
        if (leg.number.empty())
                number = concat(leg.did);
        else    number = escape_string(leg.number);

        if (b.empty())
                b = number;
        else    b = concat(b, ", ", number);

        if (leg.account_id && !r.account_id_2)
            r.account_id_2 = leg.account_id;
    }

    r.called_original_number = r.called_translated_number = r.called_e164_number = b;
    r.voip_term_cause = record->a.hangup_cause.empty() ? record->a.final_status : record->a.hangup_cause;
    r.voip_duration = parseDateTime(r.end_connection_time.c_str()) - parseDateTime(r.begin_connection_time.c_str());
    r.account_id = record->a.account_id;
    r.voip_gateways = "";
    r.voip_originator_name = escape_string(record->a.name);
    r.inbound_trunk = "";
    r.outbound_trunk = "";
    r.vpbx_id = record->a.vpbx_id;
    r.server_id = record->a.server_id;
    r.orig_uniqueid = record->a.orig;
    r.voip_protocol = "SIP";
    r.voip_data_content_id = "";
    r.telco_id = 0;
    r.point_id = 0;
    r.voip_in_bytes_count = 0;
    r.voip_out_bytes_count = 0;
    r.voip_fax = false;
    r.voip_call_type_id = 0;

    r.dump(out);
}
