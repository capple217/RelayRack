#include <iostream>
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <functional>


void print(const std::error_code& /*e*/, 
            asio::steady_timer* t, int* count) {

  if (*count < 5) {
    std::cout << *count << std::endl;
    ++(*count);

    t->expires_at(t->expiry() + asio::chrono::seconds(1));

    t->async_wait(std::bind(print, asio::placeholders::error, t, count));
  }
}

int main() {
  asio::io_context io;
  int count = 0;
  asio::steady_timer t(io, asio::chrono::seconds(1));

  t.async_wait(std::bind(print, asio::placeholders::error, &t, &count));
  io.run();

  std::cout << "Final count is " << count << std::endl;

  return 0;
}
