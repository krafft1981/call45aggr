#pragma once

#include "../systems/Exception.h"

using namespace std;

class Log {
public:
    static void info(const string &text, time_t log_time=0);
    static void error(const string &text, time_t log_time=0);
    static void notice(const string &text, time_t log_time=0);
    static void warning(const string &text, time_t log_time=0);
    static void debug(const string &text, time_t log_time=0);

    static void exception(Exception &e);

    static void flush();
};
