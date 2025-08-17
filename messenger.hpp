#pragma once

#include "common.hpp"
#include <type_traits>

namespace olc {

namespace net {

// Given a message sent of a type T, we include its size and identifier in
// header
template <typename T> struct message_header {

  T id{};
  uint32_t size = 0; // unsign-32bit since not all machines have same val for
                     // size_t; this is safer, seemingly
};

template <typename T> struct message {
  message_header<T> header{};
  std::vector<uint8_t> body; // Working with bytes

  size_t size() { return sizeof(message_header<T>) + body.size(); }

  // Override for std::cout compatibility - friend since can be used by others
  friend std::ostream &operator<<(std::ostream &os, const message<T> &msg) {
    os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size << "\n";
    return os;
  }

  // Pushed any POD-like data into the message buffer
  template <typename DataType>
  friend message<T> &operator<<(message<T> &msg, const DataType &data) {

    // Check that the type is trivially copyable
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be pushed into vector");

    // Cache current size of vector; this is where we insert data
    size_t i = msg.body.size();

    // Resize the vector by the size of the data being pushed
    msg.body.resize(msg.body.size() + sizeof(DataType));

    // Physically copy the data into the newly allocated vector space
    std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

    // Update the size in the message header
    msg.header.size = msg.size();

    // Return the target message so it can be "chained"
    // We can combine any type of message with messages of another type
    return msg;
  }

  template <typename DataType>
  friend message<T> &operator>>(message<T> &msg, DataType &data) {

    // Check that the type is trivially copyable
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be pushed into vector");

    // We cache backwards from the end of the vector
    size_t i = msg.body.size() - sizeof(DataType);

    // Physically copy the data from the vector into the user variable
    std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

    // Reducing the size of the body
    msg.body.resize(i);

    // Realign header size
    msg.header.size = msg.size();

    // Return message so it can be "chained"
    return msg;
  }
};

template <typename T> class connection;

template <typename T> struct owned_message {
  std::shared_ptr<connection<T>> remote = nullptr;
  message<T> msg;

  // Override for std::cout compatibility - friend since can be used by others
  friend std::ostream &operator<<(std::ostream &os, const message<T> &msg) {
    os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size << "\n";
    return os;
  }
};
} // namespace net
} // namespace olc
