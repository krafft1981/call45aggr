#include "PullWorker.h"
#include <systems/Exception.h>
#include <daemon/daemon.hpp>
#include <daemon/thread_name.hpp>

PullWorker::PullWorker() {
}


void PullWorker::start(PullerPriority priority) {
    thread_ = std::thread(&PullWorker::working_loop, this, priority);
}

PullWorker::~PullWorker() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    blocked_queue_.enqueue({[](){}, tl_exception_holder()});
    if (thread_.joinable()){
        thread_.join();
    }
}

void PullWorker::add(std::string const& event, PullerPtr value){
    extractors_[event] = value;
}

bool PullWorker::working() const {
    return events_count() != 0;
}

int PullWorker::events_count() const {
    return tasks_count_.load(std::memory_order_relaxed);
}


void PullWorker::copy_names_to(std::vector<std::string>& result) const{
    for (auto const& p :extractors_){
        result.push_back(p.first);
    }
}


void PullWorker::enqueue(std::string const& event, std::vector<int64_t> const& ids,
                            std::vector<int64_t> const& versions,
                            std::function<void(size_t)> delayer ){

    // TODO: Сделать перемещение в лямбду при переходе на новый стандарт
    auto routine = [this, versions, ids, event, delayer]() mutable {
        PullerPtr puller = extractors_.at(event);
        puller->pull(delayer, ids, versions);
    };

    if (blocked_queue_.enqueue({std::move(routine), tl_exception_holder()})) {
        tasks_count_.fetch_add(1, std::memory_order_relaxed);
    }
}

void PullWorker::working_loop(PullerPriority priority){
    set_current_thread_name(concat(priority, "_sync_queue"));
    while(app::running()){

        Task task;
        blocked_queue_.wait_dequeue(task);
        tasks_count_.fetch_sub(1,std::memory_order_relaxed);

        try {
            task.routine_();
        } catch(...){
            task.exception_holder_.save_exception(std::current_exception());
        }
    }
}
