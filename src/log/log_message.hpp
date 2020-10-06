#pragma once

#include <memory>
#include <iosfwd>
#include <json/json.h>

enum class log_message_level { CRITICAL, WARNING, ERROR, NOTICE, INFO, VERBOSE, DEBUG};
enum class log_message_destination { ANY = 0, SMS, VOICE, DATA};

struct log_message {
    std::string       message;
    time_t            timestamp;
    log_message_level level;
    std::string       thread_name;
    time_t            delay_in_seconds_ = 0; // waiting for second publishing log

    Json::Value       json;
    log_message_destination  destination = log_message_destination::ANY;


    log_message();
};

using log_message_ptr = std::shared_ptr<log_message>;

std::ostream& operator << (std::ostream& os, log_message_level lev);
log_message_level parse_log_level(std::string const& level);
