#pragma once

#include <map>
#include <ctime>
#include <ostream>
#include <concurrentqueue.h>
#include "data/model/CallLeg.h"
#include "daemon/thread.hpp"

class ReadCallLeg : public Thread {
public:
    ReadCallLeg(moodycamel::ConcurrentQueue<CallLeg> * output)
        :   output_(output),
            startTime_(0),
            stopTime_(0),
            paused_(false)
    {};

    void setStart(std::time_t stemp);
    std::time_t getTime();
    void statusHtml(std::ostream& out);
    void pause();
    void resume();
    bool getPaused();

private:
    void run();
    int getSQLData(std::time_t start, std::time_t stop);

    moodycamel::ConcurrentQueue<CallLeg> * output_;

    std::time_t startTime_;
    std::time_t stopTime_;
    bool paused_;

    std::map<int, int> readId_;
};
