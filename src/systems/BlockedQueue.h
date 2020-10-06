#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockedQueue {
public:

    T dequeue() {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
            cond_.wait(mlock);

        auto item = queue_.front();
        queue_.pop();
        return item;
    }

    void dequeue(T& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty() )
            cond_.wait(mlock);

        item = std::move(queue_.front());
        queue_.pop();
    }

    void dequeue_array(std::queue<T>& destination) {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty() )
            cond_.wait(mlock);

        std::swap(destination, queue_);
    }

    std::queue<T> dequeue_array(){
        std::queue<T> result;
        dequeue_array(result);
        return result;
    }

    template <typename It>
    void wait_dequeue_bulk(It it, unsigned) {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty() )
            cond_.wait(mlock);

        while (!queue_.empty() ) {
            it = queue_.front();
            queue_.pop();
            ++it;
        }
    }

    void enqueue(const T& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }

    void enqueue(T&& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(std::move(item));
        mlock.unlock();
        cond_.notify_one();
    }

    bool empty() const {
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_.empty();
    }

    size_t size() const{
        std::unique_lock<std::mutex> mlock(mutex_);
        return queue_.size();
    }

    // Cancelation token будет находиться на уровень выше
    void stop(T&& fake_object = T()){
        enqueue(fake_object);
    }

private:
    std::queue<T>           queue_;
    mutable std::mutex      mutex_;
    std::condition_variable cond_;
};
