#pragma once

#include <map>
#include <mutex>
#include <vector>
#include "../systems/Singleton.h"

enum class health_status {
    WARNING = 1,
    ERROR = 2,
    CRITICAL = 3
};

struct health_state {
    std::string item;
    std::string status;
    std::string status_message;
    health_status status_type;
    time_t last_time;
};

struct health_register : public Singleton<health_register> {

    static void warning(std::string const& item, std::string const& status_message);
    static void error(std::string const& item, std::string const& status_message);
    static void critical(std::string const& item, std::string const& status_message);

    static void clear();

    static std::vector<health_state> states();

private:

    void erase_old_messages();

    void register_message(health_status mes_type, std::string const& item, std::string const& status_message);

    std::mutex mutex_;
    std::map<std::string, health_state> values_;
};
