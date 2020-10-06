#pragma once
#include <vector>
#include <string>
#include "common.hpp"
#include "BaseModel.h"

struct DataVoipRecord : public BaseModel {

    int telco_id;
    std::string begin_connection_time;
    std::string end_connection_time;
    std::string client_info;
    std::string server_info;
    int point_id;
    std::string voip_session_id;
    std::string voip_conference_id;
    int voip_duration;
    std::string voip_originator_name;
    int voip_call_type_id;
    std::string calling_original_number;
    std::string calling_translated_number;
    std::string calling_e164_number;
    std::string called_original_number;
    std::string called_translated_number;
    std::string called_e164_number;
    long long int voip_in_bytes_count;
    long long int voip_out_bytes_count;
    bool voip_fax;
    std::string voip_term_cause;
    std::string inbound_trunk;
    std::string outbound_trunk;
    std::string voip_gateways;
    std::string voip_protocol;
    int supplement_service_id;
    std::string voip_abonent_id;
    std::string voip_nat_info;
    std::string voip_location;
    std::string voip_event;
    std::string voip_data_content_id;
    int vpbx_id;
    int server_id;
    int account_id;
    int account_id_2;
    std::string orig_uniqueid;
    std::string raw_data;

    void dump (std::ostream& trace) {
        trace << "(" << telco_id << ", ";
        trace << "'" << begin_connection_time << "', ";
        trace << "'" << end_connection_time << "', ";
        trace << "'" << client_info << "', ";
        trace << "'" << server_info << "', ";
        trace << point_id << ", ";
        trace << "'" << voip_session_id << "', ";
        trace << "'" << voip_conference_id << "', ";
        trace << voip_duration << ", ";
        trace << "'" << voip_originator_name << "', ";
        trace << voip_call_type_id << ", ";
        trace << "'" << calling_original_number << "', ";
        trace << "'" << calling_translated_number << "', ";
        trace << "'" << calling_e164_number << "', ";
        trace << "'" << called_original_number << "', ";
        trace << "'" << called_translated_number << "', ";
        trace << "'" << called_e164_number << "', ";
        trace << voip_in_bytes_count << ", ";
        trace << voip_out_bytes_count << ", ";
        trace << (voip_fax?"true":"false") << ", ";
        trace << "'" << voip_term_cause << "', ";
        trace << "'" << inbound_trunk << "', ";
        trace << "'" << outbound_trunk << "', ";
        trace << "'" << voip_gateways << "', ";
        trace << "'" << voip_protocol << "', ";
        trace << supplement_service_id << ", ";
        trace << "'" << voip_abonent_id << "', ";
        trace << "'" << voip_nat_info << "', ";
        trace << "'" << voip_location << "', ";
        trace << "'" << voip_event << "', ";
        trace << "'" << voip_data_content_id << "', ";
        trace << vpbx_id << ", ";
        trace << server_id << ", ";
        trace << account_id << ", ";
        trace << account_id_2 << ", ";
        trace << "'" << orig_uniqueid << "', ";
        trace << "'" << raw_data << "'";
        trace << ")";
    }
};
