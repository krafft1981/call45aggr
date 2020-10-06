#pragma once

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <ctime>
#include <string>
#include <ostream>
#include <concurrentqueue.h>
#include "daemon/thread.hpp"
#include "data/model/CallLeg.h"
#include "data/model/CallLegLink.h"
#include "data/model/CallRecord.h"
#include "service/ReadCallLeg.h"
#include "service/ReadCallLegLink.h"
#include "service/MergeOnline.h"

class MergeContainer : public Thread {
public:
    MergeContainer(moodycamel::ConcurrentQueue<CallRecord> * output);
    void setStart(std::time_t start);
    std::time_t getStart();
    std::time_t getMaxLegTime();
    std::time_t getMaxLinkTime();
    MergeOnline* getOnline();
    int getLegQueueSize();
    int getLinkQueueSize();
    int getLegSize();
    std::tuple<int, int> getLinkSize();
    int getDataSize();
    void legStatusHtml(std::ostream& out);
    void linkStatusHtml(std::ostream& out);

private:
    void run();
    void onStart();
    void onStop();

    std::list<CallRecord>                              data_;
    std::map<std::string, CallLeg>                     legs_;
    std::map<std::string, std::vector<CallLegLink>>   links_;

    std::deque<CallLeg>      legsIn_;
    std::deque<CallLegLink> linksIn_;

    moodycamel::ConcurrentQueue<CallLeg>           queueLeg_;
    moodycamel::ConcurrentQueue<CallLegLink>      queueLink_;
    moodycamel::ConcurrentQueue<CallRecord>   * queueOutput_;

    ReadCallLeg      legReader_;
    ReadCallLegLink  linkReader_;

    std::time_t maxLegTime_;
    std::time_t maxLinkTime_;
    std::time_t start_;
};
