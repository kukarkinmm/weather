//
// Created by michael on 20.10.2020.
//

#ifndef WEATHER_SRC_CLIENT_INCLUDE_CLIENT_CLIENT_H
#define WEATHER_SRC_CLIENT_INCLUDE_CLIENT_CLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>

class Client {
  using tcp = boost::asio::ip::tcp;

  boost::asio::io_context ioc{1};
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc.get_executor()};
  boost::asio::thread_pool worker{2};
  tcp::resolver resolver{ioc};
  boost::beast::tcp_stream stream{ioc};
  boost::asio::steady_timer waiter{ioc};
  std::string host;
  std::string port;
  int version = 11;

  boost::asio::steady_timer buffer_timer{ioc};
  boost::circular_buffer<boost::beast::http::request<boost::beast::http::string_body>> buffer{100};

  std::function<void(std::pair<bool, std::stringstream>)> callback;

  void connect(boost::asio::yield_context yield);
  void sender(boost::asio::yield_context yield);
  void reader(boost::asio::yield_context yield);

public:
  Client(std::string_view host_, std::string_view port_);
  void subscribe(std::function<void(std::pair<bool, std::stringstream>)>);
  void send(std::string target);
  ~Client();
};

#endif //WEATHER_SRC_CLIENT_INCLUDE_CLIENT_CLIENT_H
