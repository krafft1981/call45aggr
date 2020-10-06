#pragma once

// Частичная копия примера асинхронного сервера boost::beast
// Сервер поддерживает keep_alive

//------------------------------------------------------------------------------
//
// Example: HTTP server, asynchronous
//
//------------------------------------------------------------------------------

//#include <boost/beast/core.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <boost/asio/ip/tcp.hpp>
#include "web/view.hpp"

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {

  std::shared_ptr<tcp::acceptor> acceptor_;
  tcp::socket socket_;

  tcp::endpoint endpoint_;

  std::shared_ptr<view> controller_;

public:
  listener(boost::asio::io_context &ioc, tcp::endpoint endpoint,
           std::shared_ptr<view> controller);

  // Start accepting incoming connections
  void run() ;

  void do_accept() ;

  void stop() ;

  void on_accept(boost::system::error_code ec) ;
};
