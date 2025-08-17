#pragma once

#include "common.hpp"
#include "messenger.hpp"
#include "tsQueue.hpp"
#include <memory>

namespace olc {
namespace net {

template <typename T>
class connection : public std::enable_shared_from_this<T> {
public:
  connection() {}

  virtual ~connection() {}

public:
  bool ConnectToServer();
  bool Disconnect();
  bool isConnected() const;

public:
  bool Send(const message<T> &msg);

protected:
  // Each connection ahs a unique socket to a remote
  asio::ip::tcp::socket m_socket;

  // This context is shared with the whole asio instance
  asio::io_context m_asioContext;

  // This queue holds all messages to be sent to the
  // remote side of this connection
  tsqueue<message<T>> m_qMessagesOut;

  // This queue holds all messages that have been recieved from
  // the remote side of thsi connection. Note: it's a reference
  // as the "owner" of this connection is expected to provide a queue
  tsqueue<owned_message<T>> &m_qMessagesIn;
};

} // namespace net
} // namespace olc
