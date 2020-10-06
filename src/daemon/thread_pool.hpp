#pragma once

#include <map>
#include "thread.hpp"

class ThreadPool {

private:
    std::map<std::string, Thread> pool;
};
