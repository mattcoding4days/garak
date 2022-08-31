//
// Created by matt on 8/28/22.
//
// A very common implementation for wrapping stl containers for thread safety.
// Code just like this can be found literally all over the internet and on
// YouTube. The nifty little variadic argument tricks are all things I
// learned from other programmers on the internet, the genius is theirs to
// claim.

#ifndef LIBGARAK_CONTAINERS_THREAD_SAFE_DEQUEUE_HPP
#define LIBGARAK_CONTAINERS_THREAD_SAFE_DEQUEUE_HPP

#include <garak/utils/common.hpp>

namespace garak::containers {
template <class T, class Allocator = std::allocator<T>>
class ThreadSafeDequeue {
  using Mutex = std::mutex;
  using ULock = std::unique_lock<Mutex>;

 public:
  ThreadSafeDequeue() = default;
  virtual ~ThreadSafeDequeue() { clear(); }
  ThreadSafeDequeue(ThreadSafeDequeue const &) = delete;
  ThreadSafeDequeue &operator=(ThreadSafeDequeue const &) = delete;
  ThreadSafeDequeue(ThreadSafeDequeue &&) noexcept = default;
  ThreadSafeDequeue &operator=(ThreadSafeDequeue &&) noexcept = default;

  /**
   * @brief Returns a const reference to the first data element in the queue,
   * no items are removed
   * */
  [[nodiscard]] typename std::deque<T>::const_reference front() noexcept {
    ULock lock{this->mMutex};
    while (this->mInternal.empty()) {
      this->mCondVar.wait(lock);
    }
    return this->mInternal.front();
  }

  /**
   * @brief Returns a const reference to the data at the last element of
   * the %deque.
   * */
  [[nodiscard]] typename std::deque<T>::const_reference back() noexcept {
    ULock lock{this->mMutex};
    while (this->mInternal.empty()) {
      this->mCondVar.wait(lock);
    }
    return this->mInternal.back();
  }

  /**
   * @brief Inserts a new element at the end of the container.
   * The element is constructed through std::allocator_traits::construct,
   * which typically uses placement-new to construct the element in-place at
   * the location provided by the container. The arguments args... are
   * forwarded to the constructor as
   * */
  template <class... Args>
  void emplace_back(Args &&...args) {
    addDataProtected_(
        [&]() -> void { mInternal.emplace_back(std::forward<Args>(args)...); });
  }

  /**
   * @brief Inserts a new element to the beginning of the container.
   * The element is constructed through std::allocator_traits::construct,
   * which typically uses placement-new to construct the element in-place at
   * the location provided by the container. The arguments args... are
   * forwarded to the constructor as
   * */
  template <class... Args>
  void emplace_front(Args &&...args) {
    addDataProtected_([&]() -> void {
      mInternal.emplace_front(std::forward<Args>(args)...);
    });
  }

  /**
   * @brief Adds Data to the end of the %deque
   * The element is constructed through std::allocator_traits::construct,
   * which typically uses placement-new to construct the element in-place at
   * the location provided by the container. The arguments args... are
   * forwarded to the constructor as
   * */
  void push_back(T const &elem) {
    addDataProtected_([&]() -> void { mInternal.push_back(std::move(elem)); });
  }

  /**
   * @brief Add data to the front of the %deque.
   * @param elem Data to be added.
   * This is a typical stack operation. The function creates an element at the
   * front of the %deque and assigns the given data to it. Due to the nature
   * of a %deque this operation can be done in constant time.
   * */
  void push_front(T const &elem) {
    addDataProtected_([&]() -> void { mInternal.push_front(std::move(elem)); });
  }

  /**
   * @brief Removes a single element from the front of the %deque
   * This is a typical stack operation. It shrinks the %deque by one.
   * NOTE, unlike to std::deque, Data is returned
   ** */
  [[maybe_unused]] T pop_front() noexcept {
    ULock lock{this->mMutex};
    while (this->mInternal.empty()) {
      this->mCondVar.wait(lock);
    }
    auto elem = std::move(this->mInternal.front());
    this->mInternal.pop_front();
    return elem;
  }

  /**
   * @brief Removes and element from the back of the %deque
   * This is a typical stack operation. It shrinks the %deque by one.
   * NOTE: Unlike std::deque, data is returned
   * */
  [[maybe_unused]] T pop_back() noexcept {
    ULock lock{this->mMutex};
    while (this->mInternal.empty()) {
      this->mCondVar.wait(lock);
    }
    auto elem = std::move(this->mInternal.back());
    this->mInternal.pop_back();
    return elem;
  }

  /**
   * @brief capacity Returns the number of elements in the %deque.
   * */
  [[nodiscard]] typename std::deque<T>::size_type size() const noexcept {
    ULock lock{mMutex};
    return this->mInternal.size();
  }

  /**
   * @brief Returns true if the %deque is empty. (Thus begin() would equal
   * end().)
   * */
  [[nodiscard]] bool empty() noexcept {
    ULock lock{mMutex};
    return this->mInternal.empty();
  }

  /**
   * @brief Erases all the elements. Note that this function only erases the
   * elements, and that if the elements themselves are pointers, the
   * pointed-to memory is not touched in any way. Managing the pointer is the
   * user's responsibility.
   * */
  void clear() noexcept {
    ULock lock{mMutex};
    if (!this->mInternal.empty()) {
      this->mInternal.clear();
    }
  }

  /**
   * @brief Lock the queue
   *
   * @details: For (n) threads whom are attempting to use this container, said
   * Thread will be put "to sleep" if the container is empty. This stops
   * threads from long polling the container which results in very high CPU
   * consumption. This method is from the OneLoneCoder youtube channel, which
   * he discovers how not locking the data structure being used will result in
   * constant high cpu usage issues. This method should be added onto any
   * wrapped data structure.
   * */
  void wait() {
    ULock lock{this->mBlocking};
    while (this->mInternal.empty()) {
      this->mCondVar.wait(lock);
    }
  }

 private:
  /**
   * @brief Lambda wrapper which encapsulates redundant code that is used
   * for modifier methods such as <push_back, push_front, emplace_back,
   * emplace_front>
   * */
  void addDataProtected_(std::function<void()> &&func) {
    ULock lock{this->mMutex};
    func();
    lock.unlock();
    this->mCondVar.notify_one();
  }

 protected:
  std::deque<T, Allocator> mInternal;
  mutable Mutex mMutex;
  Mutex mBlocking;
  std::condition_variable mCondVar;
};
}  // namespace garak::containers

#endif  // LIBGARAK_CONTAINERS_THREAD_SAFE_DEQUEUE_HPP
