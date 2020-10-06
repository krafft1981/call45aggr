#pragma once
#include <string>
#include <json/json.h>
#include "BaseModel.h"

struct CallLegLink : public BaseModel {
    long long int          id;
    int                    server_id;
    std::string            uniqueid_orig;
    std::string            channame_orig;
    std::string            uniqueid_term;
    std::string            channame_term;
    std::string            link_start_time;
    std::string            link_answer_time;
    std::string            link_end_time;
    std::string            rec_filename;
    std::string            status;
    bool                   bridged;
    std::string            cf;
    std::string            trf_by;
    std::string            queue_callid;
    std::string            object_type;
    Json::Value            object_data;

    void dump (std::ostream& trace) {
	trace << "(";
	trace << "id: "               << id << ", ";
	trace << "server_id: "        << server_id << ", ";
	trace << "uniqueid_orig: "    << uniqueid_orig << ", ";
	trace << "channame_orig: "    << channame_orig << ", ";
	trace << "uniqueid_term: "    << uniqueid_term << ", ";
	trace << "channame_term: "    << channame_term << ", ";
	trace << "link_start_time: "  << link_start_time << ", ";
	trace << "link_answer_time: " << link_answer_time << ", ";
	trace << "link_end_time: "    << link_end_time << ", ";
	trace << "rec_filename: "     << rec_filename << ", ";
	trace << "status: "           << status << ", ";
	trace << "bridged: "          << bridged << ", ";
	trace << "cf: "               << cf << ", ";
	trace << "trf_by: "           << trf_by << ", ";
	trace << "queue_callid: "     << queue_callid << ", ";
	trace << "object_type: "      << object_type << ", ";
	trace << "object_data: "      << object_data.toStyledString() << ", ";
	trace << ")";
    };

    Json::Value json() {
	    Json::Value out;

	    out["id"]               = (int)id;
	    out["server_id"]        = (int)server_id;
	    out["uniqueid_orig"]    = uniqueid_orig;
	    out["channame_orig"]    = channame_orig;
	    out["uniqueid_term"]    = uniqueid_term;
	    out["channame_term"]    = channame_term;
	    out["link_start_time"]  = link_start_time;
	    out["link_answer_time"] = link_answer_time;
	    out["link_end_time"]    = link_end_time;
	    out["rec_filename"]     = rec_filename;
	    out["status"]           = status;
	    out["bridged"]          = bridged;
	    out["cf"]               = cf;
	    out["trf_by"]           = trf_by;
	    out["queue_callid"]     = queue_callid;
	    out["object_type"]      = object_type;
	    out["object_data"]      = object_data;

	    return out;
    };
};
