#pragma once
#include <string>
#include "BaseModel.h"

struct CallLegOnline : public BaseModel {
    std::string		uniqueid;
    std::string		channame;
    bool			orig;
    int				account_id;
    int				vpbx_id;
    std::string		start_time;
    std::string		answer_time;
    std::string		stop_time;
    std::string		final_status;
    int				q931_status;
    std::string		hangup_cause;
    std::string		number;
    std::string		name;
    long long int	did;
    std::string		did_type;
    std::string		object_type;
    int				object_id;
    std::string		sip_ip;
    std::string		last_linked_id;
    std::string		sip_call_hash_id;
    std::string		extra_info_id;

    void dump (std::ostream& trace) {
		trace << "(";
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
		trace << "extra_info_id: "    << extra_info_id << ", ";
		trace << ")";
	};
};
