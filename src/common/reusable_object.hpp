#pragma once

#include <memory>
#include <assert.h>

template<typename T>
struct reusable_object {

    // Чтобы объект был достаточно легковесным используются глобальная функции
    // Можно еще передавать функцию с аргументом.
    using callback_t = void (*)(int poolId, std::shared_ptr<T>);

    int                poolId_;
    std::shared_ptr<T> instance_;
    callback_t         callback_;

    reusable_object(callback_t cbk, int poolId, std::shared_ptr<T> instance)
            : poolId_(poolId),
            instance_(instance),
            callback_(cbk)
    {}

    reusable_object(callback_t cbk, std::shared_ptr<T> instance)
            : poolId_(0),
            instance_(instance),
            callback_(cbk)
    {}

    reusable_object(reusable_object&&) = default;
    reusable_object& operator=(reusable_object&&) = default;
    reusable_object& operator=(const reusable_object<T>&& object) = delete;
    reusable_object(const reusable_object<T>& object) 
            : poolId_(object.poolId_),
            instance_(object.instance_),
            callback_(object.callback_)
    {}

    operator bool() const {
        return static_cast<bool>(instance_);
    }

    T* operator->() {
        assert(instance_);
        return instance_.get();
    }

    T& operator*() {
        assert(instance_);
        return *instance_;
    }

    T* get() {
        return instance_.get();
    }

    ~reusable_object() {
        if (!instance_) {
            return;
        }
        callback_(poolId_, instance_);
    }
};
