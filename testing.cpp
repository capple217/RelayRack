#include <iostream>
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <functional>
#include <ctime>
#include <string>

using asio::ip::tcp;

std::string make_daytime_string() {
  using namespace std;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
  typedef std::shared_ptr<tcp_connection> pointer;
  
  static pointer create(asio::io_context& io_context) {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket() {
    return socket_;
  }

  void start() {
    message_ = make_daytime_string();

    asio::async_write(socket_, asio::buffer(message_), std::bind(&tcp_connection::handle_write,
                                                                  shared_from_this(),
                                                                  asio::placeholders::error,
                                                                  asio::placeholders::bytes_transferred));
  }

private:
  tcp::socket socket_;
  std::string message_;

  tcp_connection(asio::io_context& io_context) : socket_{io_context} {}
  void handle_write(const std::error_code& /*error*/, size_t /*bytes_transferred*/) {}
};

class tcp_server {
public:
  tcp_server(asio::io_context& io_context) : io_context_{io_context},
                                              acceptor_{io_context, tcp::endpoint(tcp::v4(), 8000)} 
    {
      start_accept();
    }

private:
    
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;

    void start_accept() {
      tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

      acceptor_.async_accept(new_connection->socket(), std::bind(&tcp_server::handle_accept,
                                                                  this, 
                                                                  new_connection, 
                                                                  asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer new_connection, const std::error_code& error) {
      if (!error) {
        new_connection->start();
      }      

      start_accept();
    }

};

int main() {
  try {
    asio::io_context io_context;
    tcp_server server(io_context);

    io_context.run();
  }

  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}

