#pragma once

#include <functional>

// Выполняет задачу при выходе из функции
template<typename RollbackFunction>
struct ScopeGuard {
    RollbackFunction task_;
    bool committed_;

    explicit ScopeGuard(RollbackFunction&& task)
            : task_(std::forward<RollbackFunction>(task)),
              committed_(false)
    {}

    ScopeGuard(ScopeGuard&& other)
            : task_(std::move(other.task_)),
              committed_(other.committed_) {
        other.committed_ = true;
    }

    ~ScopeGuard() {
        if (!committed_) {
            task_();
        }
    }

    void commit() {
        committed_ = true;
    }
};

template<typename RollbackFunction>
ScopeGuard<RollbackFunction> makeScopeGuard(RollbackFunction&& fun){
    return ScopeGuard<RollbackFunction>(std::forward<RollbackFunction>(fun));
}

using DefaultScopeGuard = ScopeGuard<std::function<void()>>;
