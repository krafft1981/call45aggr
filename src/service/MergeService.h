#pragma once

#include <ctime>
#include <sstream>
#include <concurrentqueue.h>
#include "data/model/CallRecord.h"
#include "service/MergeContainer.h"

class MergeService {
public:
    MergeService()
        : data_(&output_){};

    void run();
    MergeContainer* getData();
    int getOutputQueueSize();

private:
    MergeContainer data_;
    moodycamel::ConcurrentQueue<CallRecord> output_;
};
