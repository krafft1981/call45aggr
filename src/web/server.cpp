#include "server.hpp"
#include <common/split.h>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/config.hpp>
#include <boost/type_index.hpp>
#include <boost/bind.hpp>
#include <boost/beast/core.hpp>
#include <daemon/config.hpp>

namespace http = boost::beast::http;    // from <boost/beast/http.hpp>


bool url_decode(const std::string &in, std::string &out) {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char> (value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}



std::pair<std::string, std::string> split_by(std::string const& url, char separator){
    auto separatorPlace = url.find(separator);
    auto secondValuePlace = (separatorPlace == std::string::npos ? url.size() : separatorPlace+1);

    return std::make_pair(url.substr(0, separatorPlace),
                          url.substr(secondValuePlace));
}


std::pair<std::string, std::map<std::string,std::string>> parse_url(std::string const& url) {

    std::pair<std::string, std::map<std::string,std::string>> result;

    auto separatorPlace = url.find('?');
    if (separatorPlace == std::string::npos) {
        result.first = url;
        return result;
    }

    auto& params = result.second;
    auto key_pair_values = url.substr(separatorPlace+1);
    std::vector<std::string> sparams = split_any(key_pair_values, "&");
    for (std::string const& kv_raw : sparams){
        auto kv = split_by(kv_raw, '=');

        std::string key, value;
        url_decode(kv.first, key);
        url_decode(kv.second, value);

        params[key] = value;
    }

    result.first = url.substr(0, separatorPlace);
    return std::move(result);
}



// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template <class Body, class Allocator, class Send>
void handle_request(http::request<Body, http::basic_fields<Allocator>> &&req,
                    Send &&send, view& ctrl) {

    // Код boost::beast имеет множество зависимостей в заголовках.
    // Поэтому запросы обрабатываются через сторонние структуры данных
    http_request request;
    std::for_each(req.begin(), req.end(), [&](auto& header) {
        request.fields_.emplace_back();
        request.fields_.back().name_ = header.name();
        request.fields_.back().value_ = header.value();
    });

    // Заголовки должны находиться по lower_bound
    std::sort(request.fields_.begin(), request.fields_.end(), [](http_request::field const& lhs, http_request::field const& rhs) {
        return lhs.name_ < rhs.name_;
    });

    request.body_ = req.body();
    request.method_ = req.method();
    std::tie(request.path_, request.parameters_) = parse_url(req.target().to_string());

    // apply view just at this place
    http_response response = ctrl.handle(request);

    http::response<http::string_body> res(static_cast<http::status>(response.code_), req.version());
    res.set(http::field::server, CALL45AGGR_SERVER);
    res.set(http::field::content_type, response.mime_type_);
    res.set(http::field::content_length, std::to_string(response.body_.size()));
    res.keep_alive(req.keep_alive());
    res.body() = std::move(response.body_);

    for (auto const& field : response.fields_){
        res.insert(field.name_, field.value_);
    }

    send(std::move(res));
}

//------------------------------------------------------------------------------

// Report a failure
void fail(boost::system::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session> {
    // This is the C++11 equivalent of a generic lambda.
    // The function object is used to send an HTTP message.
    struct send_lambda {
        session &self_;

        explicit send_lambda(session &self) : self_(self) {}

        template <bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields> &&msg) const {
            // The lifetime of the message has to extend
            // for the duration of the async operation so
            // we use a shared_ptr to manage it.
            auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(
                    std::move(msg));

            // Store a type-erased version of the shared
            // pointer in the class to keep it alive.
            self_.res_ = sp;

            // Write the response
            http::async_write(self_.socket_, *sp,
                              boost::asio::bind_executor(self_.strand_, [
                                      ths = self_.shared_from_this(), eof = sp->need_eof()
                              ](boost::system::error_code ec, std::size_t bytes_trn) {
                                  ths->on_write(ec, bytes_trn, eof);
                              }));
        }
    };

    tcp::socket socket_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    send_lambda lambda_;
    std::shared_ptr<view> controller_;

public:
    // Take ownership of the socket
    explicit session(tcp::socket socket, std::shared_ptr<view> controller)
            : socket_(std::move(socket)), strand_(socket_.get_executor()),
            lambda_(*this), controller_(std::move(controller)) {
    }

    // Start the asynchronous operation
    void run() { do_read(); }

    void do_read() {

        req_ = {};
        // Read a request
        http::async_read(
                socket_, buffer_, req_,
                boost::asio::bind_executor(
                        strand_, [ths = shared_from_this()](boost::system::error_code ec,
                                                            std::size_t bytes_trn) {
                            ths->on_read(ec, bytes_trn);
                        }));
    }

    void on_read(boost::system::error_code ec, std::size_t bytes_transferred) {

        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == http::error::end_of_stream) {
            return do_close();
        }

        if (ec) {
            return fail(ec, "read");
        }

        // Send the response
        handle_request(std::move(req_), lambda_, *controller_);
    }

    void on_write(boost::system::error_code ec,
                  std::size_t bytes_transferred,
                  bool close) {
        boost::ignore_unused(bytes_transferred);

        if (ec) {
            return fail(ec, "write");
        }

        if (close) {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // We're done with the response so delete it
        res_ = nullptr;

        // Read another request
        do_read();
    }

    void do_close() {
        // Send a TCP shutdown
        boost::system::error_code ec;
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }
};

listener::listener(boost::asio::io_context &ioc, tcp::endpoint endpoint,
        std::shared_ptr<view> controller)
    :  socket_(ioc), endpoint_(endpoint), controller_(controller) {
    boost::system::error_code ec;

    std::cerr << "Start WEB server on port " << endpoint_.port() << "\n";

    tcp::resolver resolver(ioc);
    tcp::resolver::query query(endpoint.address().to_string(), std::to_string(endpoint.port()));
    acceptor_.reset(new tcp::acceptor(ioc, *resolver.resolve(query)));

}

// Start accepting incoming connections
void listener::run() {
    if (!acceptor_->is_open())
        return;
    do_accept();
}

void listener::do_accept() {
//    socket_ = tcp::socket(acceptor_.get_io_context());
    acceptor_->async_accept(
            socket_, [ths = shared_from_this()](boost::system::error_code ec) {
                ths->on_accept(ec);
            });
}

void listener::stop() {

    boost::system::error_code ec;
    acceptor_->close(ec);

//    if (!acceptor_->get_io_context().stopped()) {
//        acceptor_->get_io_context().stop();
//    }
}

void listener::on_accept(boost::system::error_code ec) {
    if (ec) {
        fail(ec, "accept");
    } else {
        // Create the session and run it
        std::make_shared<session>(std::move(socket_), controller_)->run();

    }

    // Accept another connection
    do_accept();
}


