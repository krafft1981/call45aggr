#pragma once

#include <systems/ExceptionHolder.h>
#include "Puller.h"
#include <blockingconcurrentqueue.h>
#include <unordered_map>
#include <db/BDb.h>

class PullWorker {
    struct Task{
        std::function<void()> routine_;
        ExceptionHolder       exception_holder_;
    };

    using PullerPtr = Puller const*;
    using PullerDict = std::unordered_map<std::string, PullerPtr>;
    using TaskType = Task;

public:
    PullWorker();

    ~PullWorker();

    void start(PullerPriority priority);

    void add(std::string const& event, PullerPtr value);

    bool working() const;

    void enqueue(std::string const& event, std::vector<int64_t>  const& ids,
                 std::vector<int64_t> const& versions, std::function<void(size_t)> delayer );

    // Именна всех обрабатываемых синхронизаторов
    void copy_names_to(std::vector<std::string>& result) const;

    int events_count() const;

private:
    void working_loop(PullerPriority priority);

    std::atomic_size_t     tasks_count_ = 0;
    std::thread            thread_;
    PullerDict             extractors_;
    moodycamel::BlockingConcurrentQueue<TaskType> blocked_queue_;
};
