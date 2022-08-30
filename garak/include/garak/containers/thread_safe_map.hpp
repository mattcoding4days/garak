//
// Created by matt on 8/28/22.
//
// A very common implementation for wrapping stl containers for thread safety.
// Code just like this can be found literally all over the internet and on
// YouTube. The nifty little variadic argument tricks are all things I
// learned from other programmers on the internet, the genius is theirs to
// claim.

#ifndef LIBGARAK_CONTAINERS_THREAD_SAFE_MAP_HPP
#define LIBGARAK_CONTAINERS_THREAD_SAFE_MAP_HPP

#include <garak/utils/common.hpp>

namespace garak::containers {
template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T>>>
class ThreadSafeMap {
  using Mutex = std::mutex;
  using ULock = std::unique_lock<Mutex>;

 public:
  ThreadSafeMap() = default;
  virtual ~ThreadSafeMap() { this->clear(); };
  ThreadSafeMap(ThreadSafeMap const &) = delete;
  ThreadSafeMap &operator=(ThreadSafeMap const &) = delete;
  ThreadSafeMap(ThreadSafeMap &&) noexcept = default;
  ThreadSafeMap &operator=(ThreadSafeMap &&) noexcept = default;

  /* Capacity Methods */

  /**
   * @brief Returns true if the %map is empty.  (Thus begin() would equal
   *  end().). C++ 20
   */
  [[nodiscard]] bool empty() const noexcept {
    ULock lock{mMutex};
    return this->mInternal.empty();
  }

  /**
   * @brief Returns the size of the %map.
   * */
  [[nodiscard]] typename std::map<Key, T>::size_type size() const noexcept {
    ULock lock{mMutex};
    return this->mInternal.size();
  }

  /* Modifier Methods */

  /**
   *  @brief Erases all elements in a %map.  Note that this function only
   *  erases the elements, and that if the elements themselves are
   *  pointers, the pointed-to memory is not touched in any way.
   *  Managing the pointer is the user's responsibility.
   */
  void clear() noexcept {
    ULock lock{mMutex};
    if (!this->mInternal.empty()) {
      this->mInternal.clear();
    }
  }

  /* Look Up Methods */

  /* Custom Methods */
  /**
   * @brief Lock the map
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
   * for modifier methods.
   * */
  void addDataProtected_(std::function<void()> &&func) {
    ULock lock{this->mMutex};
    func();
    lock.unlock();
    this->mCondVar.notify_one();
  }

 protected:
  std::map<Key, T, Compare, Allocator> mInternal;
  mutable Mutex mMutex;
  Mutex mBlocking;
  std::condition_variable mCondVar;
};
}  // namespace garak::containers

#endif  // LIBGARAK_CONTAINERS_THREAD_SAFE_MAP_HPP
