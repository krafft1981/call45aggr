#pragma once

#include <ctime>
#include <ostream>
#include <concurrentqueue.h>
#include "data/model/CallLegLink.h"
#include "daemon/thread.hpp"
#include "service/MergeOnline.h"

class ReadCallLegLink : public Thread {
public:
    ReadCallLegLink(moodycamel::ConcurrentQueue<CallLegLink> * output)
        :   output_(output),
            startTime_(0),
            stopTime_(0),
            paused_(false)
    {};

    void setUpperBorder(std::time_t stemp);
    void setStart(std::time_t stemp);
    std::time_t getTime();
    void statusHtml(std::ostream& out);
    MergeOnline* getOnline();
    void pause();
    void resume();
    bool getPaused();

private:
    void run();
    int getSQLData(std::time_t start, std::time_t stop);

    moodycamel::ConcurrentQueue<CallLegLink> * output_;

    MergeOnline online_;
    std::string waitLinkOrig_;
    std::string waitLinkTerm_;

    std::time_t startTime_;
    std::time_t stopTime_;
    bool paused_;
};
