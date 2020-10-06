#pragma once
#include <string>
#include <json/json.h>
#include "common.hpp"
#include "BaseModel.h"

struct CallLeg : public BaseModel {
    long long int	id;
    int			server_id;
    std::string		uniqueid;
    std::string		channame;
    bool		orig;
    int			account_id;
    int			vpbx_id;
    std::string		start_time;
    std::string		answer_time;
    std::string		stop_time;
    std::string		final_status;
    int			q931_status;
    std::string		hangup_cause;
    std::string		number;
    std::string		name;
    long long int	did;
    std::string		did_type;
    std::string		object_type;
    int		     	object_id;
    std::string		sip_ip;
    std::string		last_linked_id;
    std::string		sip_call_hash_id;
    std::string		call_id_md5;
    std::string		extra_info_id;

    void dump (std::ostream& trace) {
	trace << "(";
	trace << "id: "               << id << ", ";
	trace << "server_id: "        << server_id << ", ";
	trace << "uniqueid: "         << uniqueid << ", ";
	trace << "channame: "         << channame << ", ";
	trace << "orig: "             << orig << ", ";
	trace << "account_id: "       << account_id << ", ";
	trace << "vpbx_id: "          << vpbx_id << ", ";
	trace << "start_time: "       << start_time << ", ";
	trace << "answer_time: "      << answer_time << ", ";
	trace << "stop_time: "        << stop_time << ", ";
	trace << "final_status: "     << final_status << ", ";
	trace << "q931_status: "      << q931_status << ", ";
	trace << "hangup_cause: "     << hangup_cause << ", ";
	trace << "number: "           << number << ", ";
	trace << "name: "             << name << ", ";
	trace << "did: "              << did << ", ";
	trace << "did_type: "         << did_type << ", ";
	trace << "object_type: "      << object_type << ", ";
	trace << "object_id: "        << object_id << ", ";
	trace << "sip_ip: "           << sip_ip << ", ";
	trace << "last_linked_id: "   << last_linked_id << ", ";
	trace << "sip_call_hash_id: " << sip_call_hash_id << ", ";
	trace << "call_id_md5 "       << call_id_md5 << ", ";
	trace << "extra_info_id: "    << extra_info_id << ", ";
	trace << ")";
	};

    Json::Value json() {
		Json::Value out;

		out["id"]               = (int)id;
		out["server_id"]        = server_id;
		out["uniqueid"]         = uniqueid;
		out["channame"]         = channame;
		out["orig"]             = orig;
		out["account_id"]       = account_id;
		out["vpbx_id"]          = vpbx_id;
		out["start_time"]       = start_time;
		out["answer_time"]      = answer_time;
		out["stop_time"]        = stop_time;
		out["final_status"]     = final_status;
		out["q931_status"]      = q931_status;
		out["hangup_cause"]     = hangup_cause;
		out["number"]           = escape_string(number);
		out["name"]             = escape_string(name);
		out["did"]              = (int)did;
		out["did_type"]         = did_type;
		out["object_type"]      = object_type;
		out["object_id"]        = object_id;
		out["sip_ip"]           = sip_ip;
		out["last_linked_id"]   = last_linked_id;
		out["sip_call_hash_id"] = sip_call_hash_id;
		out["call_id_md5"]      = call_id_md5;
		out["extra_info_id"]    = extra_info_id;

		return out;
	}
};
