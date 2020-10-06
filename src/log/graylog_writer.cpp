#include <boost/asio.hpp>
#include <iostream>
#include <json/json.h>
#include "log_message.hpp"
#include "log_handler.hpp"
#include "log.hpp"
#include "graylog_writer.hpp"
#include "daemon/application.hpp"
#include "daemon/settings.hpp"

void graylog_writer::on_log(std::vector<log_message_ptr> const &messages) {

    if (messages.empty()) {
        return;
    }

    auto settings = get_settings();
    auto& graylog_host = settings.graylog_host();
    auto graylog_port = settings.graylog_port();
    auto& graylog_source = settings.graylog_source();
    if (graylog_host.empty() || graylog_port == 0) {
        return;
    }

    boost::asio::ip::tcp::socket socket(application::instance().io_context());

    // По сути то что и было раньше
    // Инициализация сокета
    {
        boost::asio::ip::address addr = boost::asio::ip::address::from_string(graylog_host);
        boost::asio::ip::tcp::endpoint endpoint(addr, graylog_port);

        boost::system::error_code error = boost::asio::error::host_not_found;
        socket.connect(endpoint, error);
        if (error) {
            Log::error("Unable connect to graylog ", graylog_host, ":", graylog_port);
            return;
        }
    }

    {
        boost::asio::streambuf buffer;
        std::ostream           out(&buffer);

        for (auto& message : messages) {

            // Игнорирование событий без каких-либо аргументов (переданное через parameters копирутся в аргументы)
            if (message->json.empty()){
                continue;
            }

            auto& params = message->json;

            params["source"] = graylog_source;
            params["server"] = to_string(settings.instance_id());
            params["timestamp"] = to_string(message->timestamp);
            params["message"] = message->message;
            params["level"] = static_cast<int>(message->level);
            // if (!message->type.empty()) {
            //    message->params["type"] = message->type;
            //}

            out << params;
            out << '\0';
        }

        boost::system::error_code ignored_error;

        boost::asio::write(socket, buffer, boost::asio::transfer_all(), ignored_error);

    }

    socket.close();
}
