#pragma once

#include "common.hpp"
#include <mutex>

namespace olc {

namespace net {

template <typename T> class tsqueue {
public:
  tsqueue() = default;

  // Don't want there to be any copies of existing queues
  tsqueue(const tsqueue<T> &) = delete;

  virtual ~tsqueue() { clear(); }

  const T &front() {
    std::scoped_lock lock(muxQ);
    return deq.front();
  }

  const T &back() {
    std::scoped_lock lock(muxQ);
    return deq.back();
  }

  void push_back(const T &item) {
    std::scoped_lock lock(muxQ);
    deq.emplace_back(std::move(item));
  }

  void push_front(const T &item) {
    std::scoped_lock lock(muxQ);
    deq.emplace_front(std::move(item));
  }

  bool empty() {
    std::scoped_lock lock(muxQ);
    return deq.empty();
  }

  size_t count() {
    std::scoped_lock lock(muxQ);
    return deq.size();
  }

  void clear() {
    std::scoped_lock lock(muxQ);
    deq.clear();
  }

  // Removed AND returns item fron the front
  T pop_front() {
    std::scoped_lock lock(muxQ);
    auto t = std::move(deq.front());
    deq.pop_front();
    return t;
  }

  T pop_back() {
    std::scoped_lock lock(muxQ);
    auto t = std::move(deq.back());
    deq.pop_back();
    return t;
  }

protected:
  std::mutex muxQ;
  std::deque<T> deq;
};

} // namespace net
} // namespace olc
