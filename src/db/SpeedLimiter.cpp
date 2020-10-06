#include "SpeedLimiter.h"
#include <cmath>
#include <systems/Timer.h>

SpeedLimiter::SpeedLimiter(double bandwidth_limit_mbits, double min_seconds_between_reper_points):
        is_enabled{ bandwidth_limit_mbits > 0.0 },
        seconds_per_byte{ 1.0 / (bandwidth_limit_mbits * 1024.0 * 1024.0 / 8.0) },
        min_seconds_between_reper_points{ min_seconds_between_reper_points }
{}

void SpeedLimiter::on_progress_updated(std::size_t bytes_read) {
    if (is_enabled == 0) return;

    total_bytes_read += bytes_read;

    // checking, that reper points is not stored too often (i.e. interval between neighbours >= "min_seconds_between_reper_points")
    const bool is_tail_to_be_override = (
            (queue_size > 1)
            && (seconds_between(penult().time, last().time) < min_seconds_between_reper_points)
    );
    if (is_tail_to_be_override == 0) {
        queue_size = std::min(std::size_t{max_queue_size}, queue_size + 1);
        tail_index = (tail_index + 1) & index_wrap_mask;
    }

    last() = {current_time(CLOCK_MONOTONIC_RAW), total_bytes_read};

    // making pause if necessary
    if (queue_size <= 1) {
        return;
    }

    const double time_to_sleep = remaining_time(first(), last());
    if (time_to_sleep <= 0.0) {
        return;
    }

    timespec requested_time;
    requested_time.tv_sec = static_cast<decltype(requested_time.tv_sec)>(std::floor(time_to_sleep));
    requested_time.tv_nsec = static_cast<decltype(requested_time.tv_nsec)>
                                (1.0e9 * (time_to_sleep - static_cast<double>(requested_time.tv_sec)));
    nanosleep(&requested_time, nullptr);
}

const SpeedLimiter::ReperPoint& SpeedLimiter::first() const {
    return queue[(tail_index - queue_size + 1) & index_wrap_mask];
}

const SpeedLimiter::ReperPoint& SpeedLimiter::penult() const {
    return queue[(tail_index - 1) & index_wrap_mask];
}

SpeedLimiter::ReperPoint& SpeedLimiter::last() {
    return queue[tail_index];
}

double SpeedLimiter::remaining_time(const ReperPoint& start, const ReperPoint& finish) const {
    return
            static_cast<double>
                (finish.total_bytes_read - start.total_bytes_read)
                * seconds_per_byte
                - seconds_between(start.time, finish.time);
}
