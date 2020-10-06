#pragma once

#include <cstddef>
#include <time.h>

class SpeedLimiter {
public:
    SpeedLimiter(double bandwidth_limit_mbits, double min_seconds_between_reper_points = 0.01);

    void on_progress_updated(std::size_t bytes_read);

private:
    static constexpr std::size_t max_queue_size = static_cast<std::size_t>(1 << 7);
    static constexpr std::size_t index_wrap_mask = max_queue_size - 1;

    struct ReperPoint {
        timespec time;
        std::size_t total_bytes_read;
    };

    const ReperPoint& first() const ;

    const ReperPoint& penult() const;

    ReperPoint& last() ;

    double remaining_time(const ReperPoint& start, const ReperPoint& finish) const ;

    const bool is_enabled;
    const double seconds_per_byte;
    const double min_seconds_between_reper_points;

    ReperPoint queue[max_queue_size] = {};
    std::size_t queue_size = 0;
    std::size_t tail_index = 0;
    std::size_t total_bytes_read = 0;
};
