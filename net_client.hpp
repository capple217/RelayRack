#pragma once

#include "common.hpp"
#include "messenger.hpp"
#include "net_connection.hpp"
#include "tsQueue.hpp"
#include <string>

namespace olc {

namespace net {

template <typename T> class client_interface {

public:
  client_interface() : m_socket(m_context) {

    // Initial the socket with teh io context
  }

  virtual ~client_interface() {
    // If the client is destroyed, always
    // try and disconnect
    Disconnect();
  }
  // Connect to the server with the hostname/ip-address and port
  bool Connect(const std::string &host, const uint16_t port) {

    try {
      // Create connection
      m_connection = std::make_unique<connection<T>>(); // TODO

      // Resolve hostname/ip-address into tangiable physical address
      asio::ip::tcp::resolver resolver(m_context);
      m_endpoints = resolver.resolve(host, std::to_string(port));

      // Tell the connection object to connect to server
      m_connection->ConnectToServer(m_endpoints);

      // Start context thread
      thrContext = std::thread([this]() { m_context.run(); });
    }

    return false;
  }

  void Disconnect() {

    if (isConnected()) {
      // If there's a connection, end it
      m_connection->Disconnect();
    }

    // Either way, end with asio context
    m_context.stop();

    // ... and its thread
    if (thrContext.joinable())
      thrContext.join();
  }

  bool isConnected() {

    if (m_connection) {
      return m_connection->isConnected();
    } else
      return false;
  }

  // Retrieve queue of messages from server
  tsqueue<owned_message<T>> &Incoming() { return m_qMessagesIn; }

protected:
  // asio context handles the data transfer...
  asio::io_context m_context;

  // ... but needs a thread of its own to execute
  // its work commands
  std::thread thrContext;

  // This is the hardware socket that is connected to the server
  asio::ip::tcp::socket m_socket;

  // The client has a single instance of a "connection" object,
  // which handles data transfer
  std::unique_ptr<connection<T>> m_connection;

  // POTENTIALLY TEMPORARY
  asio::ip::tcp::endpoint m_endpoints;

private:
  // This is the thread safe queue of incoming messages
  // from the server
  tsqueue<owned_message<T>> m_qMessagesIn;
};
} // namespace net
} // namespace olc
