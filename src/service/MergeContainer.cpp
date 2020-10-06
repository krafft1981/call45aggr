#include <iostream>
#include <sstream>
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"
#include "common.hpp"
#include "log/log_instance.hpp"
#include "log/log.hpp"
#include "MergeContainer.h"
#include "service/format/DataVoipRecordReport.h"

MergeContainer::MergeContainer(moodycamel::ConcurrentQueue<CallRecord> * output)
    :   legReader_ (&queueLeg_),
        linkReader_(&queueLink_),
        queueOutput_(output)
{}

void MergeContainer::legStatusHtml(std::ostream& out) {

    legReader_.statusHtml(out);
}

void MergeContainer::linkStatusHtml(std::ostream& out) {

    linkReader_.statusHtml(out);
}

MergeOnline* MergeContainer::getOnline() {

    return linkReader_.getOnline();
}

int MergeContainer::getLegQueueSize() {

    return queueLeg_.size_approx();
}

int MergeContainer::getLinkQueueSize() {

    return queueLink_.size_approx();
}

int MergeContainer::getLegSize() {

    return legs_.size();
}

std::tuple<int, int> MergeContainer::getLinkSize() {

    int count = 0;
    const std::lock_guard<Spinlock> lockGuard { this->lock_ };
    for(auto cll : links_)
        count += cll.second.size();
    return { links_.size(), count };
}

int MergeContainer::getDataSize() {

    return data_.size();
}

std::time_t MergeContainer::getStart() {

    return start_;
}

std::time_t MergeContainer::getMaxLegTime() {

    return maxLegTime_;
}

std::time_t MergeContainer::getMaxLinkTime() {

    return maxLinkTime_;
}

void MergeContainer::setStart(std::time_t start) {

    start_       = start;
    maxLegTime_  = start;
    maxLinkTime_ = start;

    legReader_ .setStart(maxLegTime_);
    linkReader_.setStart(maxLinkTime_);
}

void MergeContainer::onStart() {

    linkReader_.start();
    legReader_ .start();
}

void MergeContainer::onStop() {

    linkReader_.stop();
    legReader_ .stop();
}

void MergeContainer::run() {

    while(app::running() && (getStatus() == RUNNING)) {

        legsIn_.clear();
        linksIn_.clear();

        bool legsDataExist  = queueLeg_.try_dequeue_bulk(std::back_inserter(legsIn_), get_settings().max_queue_size());
        bool linksDataExist = queueLink_.try_dequeue_bulk(std::back_inserter(linksIn_), get_settings().max_queue_size());

        // Ожидание данных из очередей.
        if(!legsDataExist && !linksDataExist) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        for(auto leg : legsIn_) {
            if (leg.orig)
                    data_.push_back(CallRecord(&leg, parseDateTime(leg.start_time.c_str())));
            else    legs_[leg.uniqueid] = leg;
            std::time_t stamp = parseDateTime(leg.start_time.c_str());
            if (maxLegTime_ < stamp)
                maxLegTime_ = stamp;
        }

        for(auto link : linksIn_) {
            auto cll = links_.find(link.uniqueid_orig);
            if(cll == links_.end())
                    links_[link.uniqueid_orig].push_back(link);
            else    cll->second.push_back(link);

            std::time_t stamp = parseDateTime(link.link_start_time.c_str());
            if (maxLinkTime_ < stamp)
                maxLinkTime_ = stamp;
        }

        int recordSaveTime = maxLegTime_ - get_settings().max_call_time();

        if (abs(maxLegTime_ - maxLinkTime_) > get_settings().max_call_time() * 2) {
            if (!legReader_.getPaused() && !linkReader_.getPaused()) {
                if (maxLegTime_ > maxLinkTime_)
                        legReader_ .pause();
                else    linkReader_.pause();
            }
        }
        else {
            legReader_ .resume();
            linkReader_.resume();
        }

        // Поиск с установкой на место cll & clt & удаление устаревших CallRecord
        for(auto record = data_.begin(); record != data_.end(); ) {
            // Поиск cll
            auto cll = links_.find(record->orig);
            if(cll != links_.end()) {
                for(auto link = cll->second.begin(); link != cll->second.end(); ++ link)
                    record->links.push_back(*link);
                const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
                links_.erase(cll);
            }

            // Ищем новые clt
            for(auto link : record->links) {
                auto clt = legs_.find(link.uniqueid_term);
                if(clt != legs_.end()) {
                    record->legs[clt->second.uniqueid] = clt->second;
                    legs_.erase(clt);
                }
            }

            // Поиск и удаление CallRecord если всё что с ним связано дочитано и CallRecord пора удалять
            if((record->startTime < recordSaveTime) && (linkReader_.getTime() > (record->startTime + get_settings().max_call_time()))) {
                while (queueOutput_->size_approx() >= get_settings().max_queue_size())
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                queueOutput_->enqueue(*record);
                record = data_.erase(record);
            }
            else
                ++ record;
        }

        // Поиск и удаление устаревших CallLegs
        for (auto leg = legs_.begin(); leg != legs_.end(); ) {
            std::time_t stamp = parseDateTime(leg->second.start_time.c_str());
            if ((stamp < recordSaveTime) && (linkReader_.getTime() > stamp + get_settings().max_call_time()))
                    leg = legs_.erase(leg);
            else    ++ leg;
        }

        // Поиск и удаление устаревших CallLinks
        const std::lock_guard<Spinlock> lockGuard{ this->lock_ };
        for (auto link = links_.begin(); link != links_.end(); ) {
            std::time_t stamp = parseDateTime(link->second.front().link_start_time.c_str());
            if (stamp < recordSaveTime)
                    link = links_.erase(link);
            else    ++ link;
        }
    }
}
