#pragma once

#include <map>
#include <ostream>
#include <vector>
#include <thread>
#include <string>

#include "daemon/thread.hpp"
#include "common/reusable_object.hpp"
#include "data/model/CallLegLinkOnline.h"

class MergeOnline {
public:
    MergeOnline();
    bool checkUniqueIdOrig(std::string uniqueId);
    bool checkUniqueIdTerm(std::string uniqueId);

    int getWatcherSize();
    std::vector<CallLegLinkOnline> getData(int id);
    void statusHtml(std::ostream& out);

private:
    class OnlineWatcher : public Thread {
    public:
        OnlineWatcher(int id)
            :   id_(id),
                ready_(false)
        {}

        OnlineWatcher(const OnlineWatcher&& watcher)
            :   id_(watcher.id_),
                ready_(watcher.ready_)
        {}

        bool checkUniqueIdOrig(std::string uniqueId);
        bool checkUniqueIdTerm(std::string uniqueId);

        std::vector<CallLegLinkOnline> getData();

    private:
        void run();
        std::string formatDateTime(std::string);

        int id_;
        bool ready_;
        std::vector<CallLegLinkOnline> data_;
    };

    std::map<int, OnlineWatcher> onlineMap_;
};
