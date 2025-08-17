#pragma once

#include "common.hpp"
#include "messenger.hpp"
#include "net_connection.hpp"
#include "tsQueue.hpp"
#include <cstdint>

namespace olc {

namespace net {

template <typename T> class server_interface {
public:
  server_interface(uint16_t port)
      : m_asioAcceptor(m_asioContext,
                       asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

  virtual ~server_interface() { Stop(); }

  bool Start() {
    try {
      waitForClientConnection();

      m_threadContext = std::thread([this]() { m_asioContext.run(); });
    } catch (std::exception &e) {
      // Something prohibited the server from listening
      std::cerr << "[Server] Exception: " << e.what() << "\n";
      return false;
    }

    std::cout << "[SERVER] Started!\n";
    return true;
  }

  void Stop() {

    // Request the context to close
    m_asioContext.stop();

    // Tidy up the context thread
    if (m_threadContext.joinable())
      m_threadContext.join();

    std::cout << "[SERVER] Stopped!\n";
  }

  // ASYNC - Instruct asio to wait for connection
  void waitForClientConnection() {
    m_asioAcceptor.async_accept([this](std::error_code ec,
                                       asio::ip::tcp::socket socket) {
      if (!ec) {
        // Remote endpoint returns the ip address
        std::cout << "[SERVER] New Connection: " << socket.remote_endpoint()
                  << "\n";

        std::shared_ptr<connection<T>> newconn =
            std::make_shared<connection<T>>(connection<T>::owner::server,
                                            m_asioContext, std::move(socket),
                                            m_qMessagesIn);

        // Give the user a chance to deny connection
        if (onClientConnect(newconn)) {

          // Connection allowed, so add to the container of new connections
          m_deqConnections.push_back(std::move(newconn));

          m_deqConnections.back()->ConnectToClient(nIDCounter++);

          std::cout << "[" << m_deqConnections.back()->GetID()
                    << "] Connection Approved\n";

        } else {
          std::cout << "[-----] Connection Denied\n";
        }
      } else {
        // Error has occurred during acceptance
        std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
      }

      // Prime the asio context with more work - again simply for
      // another connection
      waitForClientConnection();
    });
  }

  // Send a message to a specific client
  void MessageClient(std::shared_ptr<connection<T>> client,
                     const message<T> &msg) {

    if (client && client->isConnected()) {
      client->Send(msg);
    } else {
      // Assume the client's disconnected
      onClientDisconnect();
      client.reset();
      m_deqConnections.erase(
          std::remove(m_deqConnections.begin(), m_deqConnections.end(), client),
          m_deqConnections.end());
    }
  }

  // Send a message to all clients, except those we choose to ignore
  void
  messageAllClients(const message<T> &msg,
                    std::shared_ptr<connection<T>> pIgnoreClient = nullptr) {

    bool bInvalidClientExists = false;

    for (auto &client : m_deqConnections) {
      // Check if client is connect
      if (client && client->isConnected()) {
        if (client != pIgnoreClient) {
          client.Send(msg);
        }
      } else {
        OnClientDisconnect(client);
        client.reset();
        bInvalidClientExists = true;
      }
    }

    if (bInvalidClientExists) {
      m_deqConnections.erase(std::remove(m_deqConnections.begin(),
                                         m_deqConnections.end(), nullptr),
                             m_deqConnections.end());
    }
  }

  // Since size_t is unsigned, -1 becomes the largest size_t val
  void Update(size_t nMaxMessages = -1) {
    size_t nMessagesCount = 0;
    while (nMessagesCount < nMaxMessages && !m_qMessagesIn.empty()) {
      // Grab the front message
      auto msg = m_qMessagesIn.pop_front();

      // Pass to message handler
      OnMessage(msg.remote, msg.msg);

      nMessagesCount++;
    }
  }

protected:
  // Called when a client connects, you can veto the connection
  // by return false
  virtual bool OnClientConnect(std::shared_ptr<connection<T>> client) {
    return false;
  }

  // Called when a client appears to have disconnected
  virtual void onClientDisconnect(std::shared_ptr<connection<T>> client) {}

  // Called when a message arrives
  virtual void OnMessage(std::shared_ptr<connection<T>> client,
                         message<T> &msg) {}

protected:
  // Thread safe queue for incoming message packets
  tsqueue<owned_message<T>> m_qMessagesIn;

  // Container of active validated connections
  std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

  // Order of declaration is important
  // It's also the order of initialization
  asio::io_context m_asioContext;
  std::thread m_threadContext;

  // These things need an asio context
  // VVV It's a socket of its own / Sockets of the connected clients
  asio::ip::tcp::acceptor m_asioAcceptor;

  // Clients will be identified in the "wider system" via an ID
  uint32_t nIDCounter = 10000;
};
} // namespace net
} // namespace olc
