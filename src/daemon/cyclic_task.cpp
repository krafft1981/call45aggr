#include "cyclic_task.hpp"
#include <log/log.hpp>
#include <common.hpp>
#include <common/transform.hpp>
#include <common/join_through.hpp>
#include <numeric>
#include <iomanip>
#include "health_register.hpp"

cyclic_task::cyclic_task(boost::asio::io_context &ctx,
                         std::function<void()> routine,
                         char const *name,
                         time_t timeout_in_ms)
        : io_ctx_(ctx),
        routine_(std::move(routine)),
        name_(name),
        timeout_in_ms_(timeout_in_ms) {}

void cyclic_task::arm() {
    timer_ = std::make_shared<boost::asio::deadline_timer>(io_ctx_);
    timer_->expires_from_now(boost::posix_time::milliseconds(timeout_in_ms_));
    timer_->async_wait([copy=shared_from_this()](boost::system::error_code const& ec) {copy->handler(ec);});
}

namespace {
struct cyclic_task_time {
    using clock_t = std::chrono::system_clock;
    using tp_t = clock_t::time_point;

    tp_t         start_;
    cyclic_task *task_;

    cyclic_task_time(cyclic_task *task) : start_(clock_t::now()), task_(task) {
        task->start_timestamp_ = std::chrono::duration_cast<std::chrono::seconds>(start_.time_since_epoch()).count();
    }

    ~cyclic_task_time() {
        auto time_in_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                              clock_t::now() - start_)
                              .count();
        task_->last_execution_time_ = time_in_ms;
        task_->execution_time_.push(time_in_ms);
        task_->start_timestamp_ = 0;
        task_->total_execution_time_ += time_in_ms;
    }
};
} // namespace

void cyclic_task::handler(boost::system::error_code const &ec) try {

    if (ec == boost::asio::error::operation_aborted) {
        timer_.reset();
        return;
    }

    cyclic_task_time _(this);

    ++total_runs_;

    routine_();

    arm();

} catch(std::exception& ex) {

    save_error(ex.what());

    Log::error("Cyclic task ", name_, " exception: ", ex.what(), log_delay<120>());
    arm();
} catch (...) {

    save_error("Unknown");

    Log::error("Unknown exception in cyclic task ", name_, log_delay<120>());
    arm();
}

cyclic_task_info cyclic_task::info() const {
    return cyclic_task_info{name_,timeout_in_ms_, last_execution_time_, total_execution_time_, start_timestamp_, total_runs_, errors_count_};
}

void cyclic_task::dump_html(std::ostream &os) {

    os << "<table border=0>\n";
    os << "<tr>";
    os << "<td style='text-align: right' nowrap>launched:</td>";
    os << "<td style='text-align: left' nowrap>" << total_runs_ << "</td>";
    os << "</tr>";

    os << "<tr>";
    os << "<td style='text-align: right' nowrap>errors:</td>";
    os << "<td style='text-align: left' nowrap>" << errors_count_ << "</td>";
    os << "</tr>";

    std::vector<double> execution_time;
    execution_time_.copy_to(std::back_inserter(execution_time));

    double total_time = 0;
    for (auto& d : execution_time) {
        d = d/1000;
        total_time += d;
    }

    if (execution_time.size() > 0 ) {
        os << "<tr>";
        os << "<td style='text-align: right' nowrap>exec time history (ms):</td>";
        os << "<td style='text-align: left' nowrap>";
        os << join_through(execution_time, ", ");
        os << "</td>";
        os << "</tr>";


        os << "<tr>";
        os << "<td style='text-align: right' nowrap>avg exec time (ms):</td>";
        os << "<td style='text-align: left' nowrap>";
        os <<  total_time/execution_time.size();
        os << "</td>";
        os << "</tr>";


        os << "<tr>";
        os << "<td style='text-align: right' nowrap>last exec time (ms):</td>";
        os << "<td style='text-align: left' nowrap>";
        os << static_cast<double>(last_execution_time_)/1000;
        os << "</td>";
        os << "</tr>";
    }

    // Not implemented yet
    if (html_dump_){
        html_dump_(os);
    }

    auto errors = current_errors();

    if (errors.empty()){
        return;
    }

    os << R"(
<table border="1">
 <caption>Error's log</caption>
 <tr>
    <th>time</th>
    <th>text</th>
 <tr>
)";

    for (auto& err : errors) {
        err.message_ = (err.count_ == 1 ? err.message_ : concat("[x",err.count_,"] ", err.message_));
        os << "<tr>";
        os << "<td>" << string_time(err.timestamp_) << "</td>";
        os << "<td>" << err.message_ << "</td>";
        os << "</tr>";
    }

    os << "</table>";

}

void cyclic_task::save_error(std::string const &text) {

    health_register::error(name_, text);

    // Здесь не изменяется контейнер. Счетчик может инкрементироваться
    // только в этом месте, следовательно блокировка не нужна.
    if (errors_.size() > 0 && errors_.back().message_ == text) {
        ++errors_.back().count_;
        errors_.back().timestamp_ = time(nullptr);
        return;
    }

    ++errors_count_;

    {
        std::lock_guard<std::mutex> lock(errors_mutex_);
        errors_.push({time(nullptr), text, 1});
    }
}

std::vector<error_info> cyclic_task::current_errors() {
    std::vector<error_info> result;
    {
        std::lock_guard<std::mutex> lock(errors_mutex_);
        errors_.copy_to(std::back_inserter(result));
    }
    std::reverse(result.begin(), result.end());
    return result;
}
