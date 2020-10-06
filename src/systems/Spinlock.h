#pragma once

#include <atomic>
#include <thread>
#include <mutex>

class Spinlock {
public:
    Spinlock() noexcept = default;
    ~Spinlock() noexcept = default;
    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() {
        for (int spin = SPIN_BEFORE_YIELD; is_locked.test_and_set(std::memory_order_acquire); ) {
            if (spin) --spin;
            else std::this_thread::yield();
        }
    }

    bool try_lock () {
            return !is_locked.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        is_locked.clear(std::memory_order_release);
    }

private:
    std::atomic_flag is_locked = ATOMIC_FLAG_INIT;
    const int SPIN_BEFORE_YIELD = 4000;
};
