//
// Created by michael on 20.10.2020.
//

#include <iostream>
#include "client/Client.h"

Client::Client(std::string_view host_, std::string_view port_) : host{host_}, port{port_} {
  boost::system::error_code ec;
  stream.expires_never();
  boost::beast::get_lowest_layer(stream).expires_never();
  stream.socket().set_option(boost::asio::socket_base::reuse_address(true), ec);

  boost::asio::post([this]{ ioc.run(); });
  boost::asio::spawn(ioc, [this](auto yield) { connect(yield); });
}

void Client::connect(boost::asio::yield_context yield) {
  while (true) {
    boost::beast::error_code ec;
    auto results = resolver.async_resolve(host, port, yield[ec]);
    stream.async_connect(results, yield[ec]);
    if (ec) {
      waiter.expires_after(std::chrono::milliseconds {100});
      waiter.async_wait(yield[ec]);
      continue;
    } else {
      boost::asio::spawn(ioc, [this](auto yield) {
        reader(yield);
      });
      boost::asio::spawn(ioc, [this](auto yield) {
        sender(yield);
      });
      break;
    }
  }
}

void Client::sender(boost::asio::yield_context yield) {
  boost::beast::error_code ec;
  while (stream.socket().is_open()) {
    if (buffer.empty()) {
      boost::system::error_code ec1;
      buffer_timer.expires_from_now(std::chrono::steady_clock::duration::max());
      buffer_timer.async_wait(yield[ec1]);
      continue;
    }

    auto request = std::move(buffer.back());
    request.set(boost::beast::http::field::host, host);
    request.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    buffer.pop_back();

    boost::beast::http::async_write(stream, request, yield[ec]);
  }
}

void Client::reader(boost::asio::yield_context yield) {
  boost::beast::error_code ec;
  boost::beast::flat_buffer b;
  boost::beast::http::response<boost::beast::http::string_body> response;
  while (stream.socket().is_open()) {
    boost::beast::http::async_read(stream, b, response, yield[ec]);
    if (ec) {
      stream.socket().shutdown(tcp::socket::shutdown_both, ec);
      break;
    }
    boost::asio::post(worker, [this, response = std::move(response)] () mutable {
      std::stringstream ss{};
      bool result = false;
      if (response.result() == boost::beast::http::status::ok)
        result = true;
      callback({result, std::stringstream{response.body()}});
    });
  }
}

void Client::subscribe(std::function<void(std::pair<bool, std::stringstream>)> f) {
  boost::asio::post(ioc, [this, f = std::move(f)]  {
    callback = f;
  });
}

void Client::send(std::string target) {
  boost::asio::post(ioc, [this, target = std::move(target)] () mutable {
    boost::beast::http::request<boost::beast::http::string_body> request{
        boost::beast::http::verb::get,
        target,
        version
    };
    buffer.push_front(request);
    buffer_timer.cancel();
  });
}

Client::~Client() {
  stream.release_socket();
  stream.close();
  ioc.stop();
  worker.join();
}
