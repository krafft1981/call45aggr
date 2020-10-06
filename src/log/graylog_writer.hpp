#pragma once

#include "log_handler.hpp"

struct graylog_writer : public log_handler {

    virtual void on_log(std::vector<log_message_ptr> const &mes) override;
};