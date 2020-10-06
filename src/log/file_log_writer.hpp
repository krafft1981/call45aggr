#pragma once

#include "log_handler.hpp"
#include <fstream>

struct file_log_writer : public log_handler {

    file_log_writer(std::string const& path, log_message_destination dst);

    virtual void on_log(std::vector<log_message_ptr> const& mes) override;

    void write_single_message(std::ostream& out, log_message_ptr const& mes);

private:
    std::string path2file_;
    log_message_destination  destination_;
};
