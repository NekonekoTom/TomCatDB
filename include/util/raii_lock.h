#ifndef RAII_LOCK_H_
#define RAII_LOCK_H_

#include "base.h"

// Resource-Acquisition-Is-Initialization Lock
// This class is a wrapper for std::mutex that ensures the mutex
// to be safely unlocked after deconstruction.
class RAIILock {
 public:
  RAIILock(std::mutex& mutex) : mutex_(mutex) {}

  RAIILock(const RAIILock&) = delete;
  const RAIILock& operator=(const RAIILock&) = delete;

  ~RAIILock() { mutex_.unlock(); }

  void Lock() { mutex_.lock(); }
  void Unlock() { mutex_.unlock(); }

 private:
  std::mutex& mutex_;
};

#endif