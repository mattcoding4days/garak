//
// Created by matt on 8/28/22.
//
#include <gtest/gtest.h>

#include <garak/containers/module.hpp>

using namespace garak::containers;

constexpr auto SIZE = 32;

TEST(BasicDequeue, TestThreadSafeDequeueOperations) {
  ThreadSafeDequeue<int> queue{};
  for (std::size_t i = 0; i < SIZE; ++i) {
    queue.push_back(static_cast<int>(i));
  }
  ASSERT_EQ(SIZE, queue.size());

  while (!queue.empty()) {
    auto before = queue.size();
    const auto &front = queue.front();
    ASSERT_EQ(front, queue.pop_front());
    ASSERT_EQ((before - 1), queue.size());
  }
}

TEST(DeadLockDequeue, TestThreadSafeDequeueThreadingOperations) {
  ThreadSafeDequeue<int> queue{};
  constexpr std::size_t mod = 100000;
  auto populate = [&]() -> void {
    for (std::size_t i = 0; i < (SIZE * mod); ++i) {
      queue.emplace_back(i);
    }
  };

  auto depopulate = [&]() -> void {
    while (!queue.empty()) {
      [[maybe_unused]] auto front = queue.pop_front();
    }
  };
  std::thread producer(populate);
  std::thread consumer(depopulate);
  producer.join();
  consumer.join();

  if (queue.empty()) {
    ASSERT_EQ(queue.size(), 0);
  }
}

TEST(BasicMap, TestThreadSafeMapOperations) {
  ThreadSafeMap<int, std::string> map{};
}