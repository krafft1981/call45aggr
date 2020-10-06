#include "log_message.hpp"
#include <ostream>

log_message::log_message()
    : timestamp(0)
{}


std::ostream& operator << (std::ostream& os, log_message_level lev){
    switch(lev) {
        case log_message_level::DEBUG: os << "DEBUG"; break;
        case log_message_level::VERBOSE: os << "VERBOSE"; break;
        case log_message_level::INFO: os << "INFO"; break;
        case log_message_level::NOTICE: os << "NOTICE"; break;
        case log_message_level::WARNING: os << "WARNING"; break;
        case log_message_level::ERROR: os << "ERROR"; break;
        case log_message_level::CRITICAL: os << "CRITICAL"; break;
    }
    return os;
}

log_message_level parse_log_level(std::string const &level) {

    if (level == "DEBUG") {
        return log_message_level::DEBUG;
    }
    if (level == "VERBOSE") {
        return log_message_level::VERBOSE;
    }
    if (level == "INFO") {
        return log_message_level::INFO;
    }
    if (level == "WARNING") {
        return log_message_level::WARNING;
    }
    if (level == "NOTICE") {
        return log_message_level::NOTICE;
    }
    if (level == "ERROR") {
        return log_message_level::ERROR;
    }
    if (level == "CRITICAL") {
        return log_message_level::CRITICAL;
    }

    return log_message_level::INFO;
}
