#pragma once

#include <string>
#include <json/json.h>
#include "BaseModel.h"

struct LegsAggrRawRecord : public BaseModel {

    std::string start_time;
    std::string stop_time;
    std::string uniqueid;
    std::string object_type;
    std::string object_type_l;
    std::string sip_ip;
    std::string sip_ip_l;
    std::string status;
    std::string call_id_md5;
    std::string call_id_md5_l;
    Json::Value to_jsonb;

    void dump (std::ostream& trace) {
        trace << "(";
        trace << "'" << start_time << "', ";
        trace << "'" << stop_time << "', ";
        trace << "'" << uniqueid << "', ";
        trace << "'" << object_type << "', ";
        trace << "'" << object_type_l << "', ";
        trace << "'" << sip_ip << "', ";
        trace << "'" << sip_ip_l << "', ";
        trace << "'" << status << "', ";
        trace << "'" << call_id_md5 << "', ";
        trace << "'" << call_id_md5_l << "', ";
        trace << "'" << to_jsonb.toStyledString() << "'";
        trace << ")";
    }
};
