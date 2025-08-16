#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <system_error>
#include <thread>

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket &socket) {
  socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
                         [&](std::error_code ec, size_t length) {
                           if (!ec) {
                             std::cout << "\n\nRead " << length << " bytes\n\n";

                             for (auto i = 0; i < length; ++i) {
                               std::cout << vBuffer[i];
                             }

                             GrabSomeData(socket);
                           }
                         }

  );
}

int main() {

  asio::error_code ec;
  asio::io_context io;

  // Give some fake task for asio so context doesn't finish
  // NOT FINISHED SINCE ORIGINAL COMMAND IS DEPRECATED

  // Start the context
  std::thread thrContext = std::thread([&]() { io.run(); });

  asio::ip::tcp::endpoint endpoint(asio::ip::make_address("1.1.1.1"), 80);

  asio::ip::tcp::socket socket(io);

  socket.connect(endpoint, ec);

  if (!ec) {
    std::cout << "Success\n";
  } else {
    std::cout << "Failed to connect to address: " << ec.message() << "\n";
    return 0;
  }

  if (socket.is_open()) {

    GrabSomeData(socket);

    std::string sRequest = "GET /index.html HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "Connection: close\r\n\r\n";

    socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);
  }

  return 0;
}
