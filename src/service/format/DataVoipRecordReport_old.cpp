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
    CallLeg orig;

    Json::Value json;

    r.account_id = r.account_id_2 = 0;
    for(auto leg : record->legs) {
        if (leg.second.last_linked_id.size()) {
            auto it = record->legs.find(leg.second.last_linked_id);
            CallLeg data;
            if (it == record->legs.end()) {
//                std::cerr << "not found leg: " << leg.second.last_linked_id << std::endl;
            }
            else
                data = it->second;

            json["last_linked_id"] = data.json();
        }

        if (leg.second.orig) {
            orig = leg.second;
            r.begin_connection_time = orig.start_time;
            r.end_connection_time = orig.stop_time;

            if (leg.second.account_id)
                    r.account_id = leg.second.account_id;
            else    r.account_id = 0;

            r.orig_uniqueid = orig.uniqueid;

            json["orig"] = leg.second.json();

            for (auto link : record->links) {
                Json::Value jsonLink = link.json();
                auto it = record->legs.find(link.uniqueid_term);
                if (it != record->legs.end())
                    jsonLink["leg"] = it->second.json();
                json["orig"]["link"].append(jsonLink);
            }

            r.voip_term_cause = leg.second.hangup_cause.size() ? leg.second.hangup_cause : leg.second.final_status;
            continue;
        }

        if (!r.account_id_2 && r.account_id && leg.second.account_id && (r.account_id != leg.second.account_id))
            r.account_id_2 = leg.second.account_id;

        if (parseDateTime(leg.second.stop_time.c_str()) > parseDateTime(r.end_connection_time.c_str()))
            r.end_connection_time = leg.second.stop_time;

        r.client_info = concat(r.client_info, ", ", leg.second.sip_ip);

        std::string number;
        if (leg.second.number.size())
                number = escape_string(leg.second.number);
        else    number = concat(leg.second.did);

        if (r.called_e164_number.empty())
                r.called_e164_number = number;
        else    r.called_e164_number = concat(r.called_e164_number, ", ", number);

        if (r.called_original_number.empty())
                r.called_original_number = number;
        else    r.called_original_number = concat(r.called_original_number, ", ", number);

        if (r.called_translated_number.empty())
                r.called_translated_number = number;
        else    r.called_translated_number = concat(r.called_translated_number, ", ", number);
    }

    r.client_info = orig.sip_ip;
    r.voip_originator_name = escape_string(orig.name);

    std::string number;
    if (orig.number.size())
            number = escape_string(orig.number);
    else    number = concat(orig.did);

    r.calling_original_number = number;
    r.calling_e164_number = number;
    r.calling_translated_number = number;

    if (orig.object_type == "pstn")
        r.voip_event = "incoming";

    for(auto link : record->links) {
        if (link.uniqueid_orig == orig.uniqueid) {
            if ((orig.object_type != "pstn") && (orig.object_type != "webcall") && (orig.object_type != "missed_call_callback") && (record->legs[link.uniqueid_term].object_type != "pstn"))
                r.voip_event = "internal";

            if (record->legs[link.uniqueid_term].object_type == "pstn")
                r.voip_event = "outgoing";
        }
    }

    r.voip_duration = parseDateTime(r.end_connection_time.c_str()) - parseDateTime(r.begin_connection_time.c_str());

    if (r.voip_event.empty() == 0)
        r.voip_event = "uncknown";

    r.raw_data = json.toStyledString();

    r.vpbx_id = orig.vpbx_id;
    r.server_id = orig.server_id;
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
