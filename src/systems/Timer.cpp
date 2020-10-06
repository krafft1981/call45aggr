#include "Timer.h"

#include <cmath>
#include <cstdio>
#include <limits>

namespace {

std::string relativeCpuUsageStr(double cpuTime, double totalTime) {
    const double usage = cpuTime / totalTime;
    if (std::isfinite(usage)) {
        const int usageInPercents = static_cast<int>(std::round(usage * 100.0));
        return std::to_string(usageInPercents) + "%"; // to clamp one can use std::min(std::max(0, usageInPercents), 100)
    }
    return "?";
}

// returns string like "1 h 2 m " and amount of corresponding seconds to that string
std::pair<std::string, int> extract_derived_units_time_str(const int seconds) {
    struct TimeUnit {
        int duration;
        char unit_letter;
    };
    constexpr int minute_duration = 60;
    constexpr int hour_duration = 60 * minute_duration;
    constexpr int day_duration = 24 * hour_duration;
    constexpr TimeUnit time_units[] = {{day_duration, 'd'}, {hour_duration, 'h'}, {minute_duration, 'm'}};

    std::string units_str;
    int rest_seconds = seconds;
    for (const TimeUnit time_unit : time_units) {
        const int n = rest_seconds / time_unit.duration;
        if (n > 0) {
            units_str += std::to_string(n) + ' ' + time_unit.unit_letter + ' ';
            rest_seconds -= n * time_unit.duration;
        }
    }

    return { units_str, seconds - rest_seconds };
}

} // namespace

timespec current_time(clockid_t clockId) {
    timespec result = {};
    if (clock_gettime(clockId, &result) != 0) {
        throw std::runtime_error{ "failed clock_gettime" };
    };
    return result;
}

double seconds_between(const timespec& start, const timespec& finish) {
    static_assert(std::numeric_limits<decltype(start.tv_sec)>::is_signed, "seconds must be signed for subtraction");
    static_assert(std::numeric_limits<decltype(start.tv_nsec)>::is_signed, "nanoseconds must be signed for subtraction");
    return
        static_cast<double>(finish.tv_sec - start.tv_sec)
        + 1.0e-9 * static_cast<double>(finish.tv_nsec - start.tv_nsec);
}

std::string seconds_to_human_readable_str(int seconds) {
    const auto prefix = extract_derived_units_time_str(seconds);
    return prefix.first + std::to_string(seconds - prefix.second) + " s";
}

std::string seconds_to_human_readable_str(double seconds) {
    const auto prefix = extract_derived_units_time_str(static_cast<int>(seconds));
    const double rest_seconds = std::ceil((seconds - static_cast<double>(prefix.second)) * 1.0e3) * 1.0e-3;

    char seconds_str[32];
    std::snprintf(seconds_str, sizeof(seconds_str), "%.3f s", rest_seconds);

    return prefix.first + seconds_str;
}

std::string Timer::currentStatus() const {
    if (!isStarted()) {
        return "idle";
    }
    return std::string{ "started "} + seconds_to_human_readable_str(currentMeasuringInterval()) + " ago";
}

std::string Timer::lastMeasuredIntervalStr() const {
    if (nDoneMeasurements() == 0) {
        return "?";
    }
    return seconds_to_human_readable_str(lastMeasuredInterval());
}

std::string Timer::totalMeasuredIntervalsStr() const {
    return seconds_to_human_readable_str(totalMeasuredIntervals());
}

std::string Timer::totalConsumedCPUTimeStr() const {
    return seconds_to_human_readable_str(totalConsumedCPUTime());
}

std::string Timer::averageCPULoadStr() const {
    return relativeCpuUsageStr(totalConsumedCPUTime(), totalMeasuredIntervals());
}
