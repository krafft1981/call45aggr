#include "file_log_writer.hpp"
#include <common.hpp>
#include <fstream>
#include <daemon/settings.hpp>


/*
void file_log_writer::on_log(log_message const& mes) {
    std::string path = get_settings().path_to_logs();
    if (path.empty()) {
        return;
    }

    std::ofstream out(path, std::ios::app);
    write_single_message(out, mes);
    out.flush();
}*/

void file_log_writer::on_log(std::vector<log_message_ptr> const &messages) {

    if (messages.empty()) {
        return;
    }

    if (path2file_.empty()) {
        return;
    }

    std::ofstream out(path2file_, std::ios::app);
    for (auto const& m : messages) {
        // Разделение логов
        if (m->destination != destination_) {
            continue;
        }
        write_single_message(out, m);
    }
    out.flush();
}

void file_log_writer::write_single_message(std::ostream& out, log_message_ptr const &mes) {
    out << "[" << string_time(mes->timestamp) << "] [" << mes->thread_name << "] [" << mes->level << "]    " << mes->message << "\n";
}

file_log_writer::file_log_writer(std::string const &path, log_message_destination dest)
        : path2file_(path), destination_(dest)
{}
