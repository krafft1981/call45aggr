#pragma once

#include <db/SpeedLimiter.h>
#include <mutex>
#include <memory>

class SharedSpeedLimiter : public SpeedLimiter {
    std::mutex mutex_;
public:

    SharedSpeedLimiter(double bandwidth_limit_mbits, double min_seconds_between_reper_points = 0.01)
            : SpeedLimiter(bandwidth_limit_mbits, min_seconds_between_reper_points)
    {}

    ~SharedSpeedLimiter(){}

    void on_progress_updated(size_t bytes_count){
        std::lock_guard<std::mutex> lock(mutex_);
        SpeedLimiter::on_progress_updated(bytes_count);
    }
};

using SpeedLimiterPtr = std::shared_ptr<SharedSpeedLimiter>();
