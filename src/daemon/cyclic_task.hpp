#pragma once

#include <functional>
#include <memory>
#include <boost/asio.hpp>
#include <systems/RingBuffer.h>

struct cyclic_task_info {
    std::string name;
    time_t period;
    time_t last_execution_time_; // in us
    time_t total_execution_time_; // in us
    time_t start_timestamp_;
    size_t exec_count_;
    size_t errors_count_;
};

struct error_info {
    time_t timestamp_;
    std::string message_;
    size_t count_;
};

struct cyclic_task : std::enable_shared_from_this<cyclic_task> {

    boost::asio::io_context &io_ctx_;
    std::function<void()>    routine_;
    std::function<void(std::ostream&)> html_dump_;
    std::string              name_;
    time_t                   timeout_in_ms_;
    std::shared_ptr<boost::asio::deadline_timer> timer_;
    time_t start_timestamp_ = 0;

    std::mutex                  errors_mutex_;
    RingBuffer<error_info, 10>  errors_;
    size_t                      errors_count_ = 0;
    size_t                      total_runs_ = 0;
    time_t                      last_execution_time_ = 0;
    time_t                      total_execution_time_ = 0;
    RingBuffer<time_t, 10>      execution_time_;

    cyclic_task(boost::asio::io_context &ctx, std::function<void()> routine,
                char const *name, time_t timeout_in_ms);

    void arm();

    void stop() {
        timer_->cancel();
    }

    void handler(boost::system::error_code const& ec);

    void dump_html(std::ostream& os);

    cyclic_task_info info() const;

    void save_error(std::string const& text);
    std::vector<error_info> current_errors();

};
