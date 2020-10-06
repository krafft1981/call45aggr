#pragma once

#include <ctime>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

timespec current_time(clockid_t clockId);
double seconds_between(const timespec& start, const timespec& finish);
std::string seconds_to_human_readable_str(int seconds);
std::string seconds_to_human_readable_str(double seconds);

namespace impl {

template <clockid_t... clockIds> struct Timer;

template <> struct Timer<> {
    enum class State { STARTED, STOPPED };

    void start() {
        if (state != State::STOPPED) {
            throw std::logic_error{ "can't start timer from not STOPPED state" };
        }
        state = State::STARTED;
        startedFrom = std::this_thread::get_id();
    }

    void stop() {
        if (state != State::STARTED) {
            throw std::logic_error{ "can't stop timer from not STARTED state" };
        }
        if (startedFrom != std::this_thread::get_id()) {
            throw std::logic_error{ "can't start/stop timer from different threads" };
        }
        state = State::STOPPED;
        startedFrom = {};
        ++nDoneMeasurements;
    }

    bool isStarted() const {
        return state == State::STARTED;
    }

    State state = State::STOPPED;
    std::thread::id startedFrom; // какой thread запустил этот таймер. Останавливать таймер должен этот же thread
    unsigned long long nDoneMeasurements = 0;
};

template <clockid_t clockId, clockid_t... restClockIds>
struct Timer<clockId, restClockIds...> : public Timer<restClockIds...>
{
    using Next = Timer<restClockIds...>;
    using Base = impl::Timer<>;

    void start() {
        Next::start();
        startPoint = current_time(clockId);
    }

    void stop() {
        lastMeasuredInterval = currentMeasuringInterval();
        totalMeasuredIntervals += lastMeasuredInterval;
        startPoint = {};
        Next::stop();
    }

    double currentMeasuringInterval() const {
        if (!Base::isStarted()) {
            throw std::logic_error{"Timer::currentMeasuredInterval: timer is not started"};
        }
        return seconds_between(startPoint, current_time(clockId));
    }

    double lastMeasuredInterval = 0.0;
    double totalMeasuredIntervals = 0.0;
    timespec startPoint = {};
};

} // namespace impl

class Timer:
    private impl::Timer<CLOCK_THREAD_CPUTIME_ID, CLOCK_MONOTONIC_RAW>
{
    using Next = impl::Timer<CLOCK_THREAD_CPUTIME_ID, CLOCK_MONOTONIC_RAW>;
    using Base = impl::Timer<>;
    using MonotonicTimer = impl::Timer<CLOCK_MONOTONIC_RAW>;
    using ThreadCPUTimer = impl::Timer<CLOCK_THREAD_CPUTIME_ID, CLOCK_MONOTONIC_RAW>;

public:
    using Next::start; // Вызовы start/stop должны быть парными, без вложенных вызовов
    using Next::stop; // Для автоматизации вызова start/stop можно использовать TimerScope или TimerScopeGuarded, основанные на RAII idiom
    using Base::isStarted;

    // Количество полных циклов start/stop
    unsigned long long nDoneMeasurements() const {
        return Base::nDoneMeasurements;
    }

    // Если таймер запущен, то возвращает время, прошедшее с начала запуска до настоящего момента
    using MonotonicTimer::currentMeasuringInterval;

    double lastMeasuredInterval() const {
        return MonotonicTimer::lastMeasuredInterval;
    }

    double totalMeasuredIntervals() const {
        return MonotonicTimer::totalMeasuredIntervals;
    }

    double totalConsumedCPUTime() const {
        return ThreadCPUTimer::totalMeasuredIntervals;
    }

    std::string currentStatus() const;
    std::string lastMeasuredIntervalStr() const;
    std::string totalMeasuredIntervalsStr() const;
    std::string totalConsumedCPUTimeStr() const;
    std::string averageCPULoadStr() const;
};

class TimerScope {
public:
    TimerScope(Timer& timer): _timer{ &timer } {
        _timer->start();
    }

    ~TimerScope() {
        _timer->stop();
    }

private:
    TimerScope(const TimerScope&) = delete;
    TimerScope(TimerScope&&) = delete;
    TimerScope& operator=(const TimerScope&) = delete;
    TimerScope& operator=(TimerScope&&) = delete;

    Timer* const _timer;
};

template <typename Lock>
class TimerScopeGuarded {
public:
    TimerScopeGuarded(Lock& lock, Timer& timer): _lock{ &lock }, _timer{ &timer } {
        const std::lock_guard<Lock> lockGuard{ *_lock };
        _timer->start();
    }

    ~TimerScopeGuarded() {
        const std::lock_guard<Lock> lockGuard{ *_lock };
        _timer->stop();
    }

private:
    TimerScopeGuarded(const TimerScopeGuarded&) = delete;
    TimerScopeGuarded(TimerScopeGuarded&&) = delete;
    TimerScopeGuarded& operator=(const TimerScopeGuarded&) = delete;
    TimerScopeGuarded& operator=(TimerScopeGuarded&&) = delete;

    Lock* const _lock;
    Timer* const _timer;
};
