#pragma once
#include <string>
#include "BaseModel.h"

struct PstnRecord : public BaseModel {

    int telco_id;
    std::string begin_connection_time;
    int duration;
    std::string call_type_id;
    std::string supplement_service_id;
    std::string in_abonent_type;
    std::string out_abonent_type;
    std::string switch_id;
    std::string inbound_trunk;
    std::string outbound_trunk;
    std::string term_cause;
    std::string in_info;
    std::string dialed_digits;
    std::string out_info;
    std::string forwarding_identifier;
    std::string message;
    int data_content_id;

    void dump (std::ostream& trace) {
        trace << "('" << telco_id << "', ";
        trace << "'" << begin_connection_time << "', ";
	    trace << duration << ", ";
	    trace << "'" << call_type_id << "', ";
	    trace << "'" << supplement_service_id << "', ";
	    trace << "'" << in_abonent_type << "', ";
	    trace << "'" << out_abonent_type << "', ";
	    trace << "'" << switch_id << "', ";
	    trace << "'" << inbound_trunk << "', ";
	    trace << "'" << outbound_trunk << "', ";
	    trace << "'" << term_cause << "', ";
	    trace << "'" << in_info << "', ";
	    trace << "'" << dialed_digits << "', ";
	    trace << "'" << out_info << "', ";
	    trace << "'" << forwarding_identifier << "', ";
	    trace << "'" << message << "', ";
	    trace << data_content_id;
        trace << ")";
    }
};
