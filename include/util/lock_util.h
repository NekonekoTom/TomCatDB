#ifndef RAII_LOCK_H_
#define RAII_LOCK_H_

#include "base.h"
#include "condition_variable"

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

// A simple wrapper for writer-prior read-write lock
class ReadWriteLock {
 public:
  enum WakeUpOption { AllowSpuriousWakeUp, DisallowSpuriousWakeUp };

  ReadWriteLock() = default;

  ReadWriteLock(const ReadWriteLock&) = delete;
  ReadWriteLock& operator=(const ReadWriteLock&) = delete;

  ~ReadWriteLock() = default;

  void ReadLock();
  void ReadUnlock();
  void WriteLock(WakeUpOption option = AllowSpuriousWakeUp);
  void WriteUnlock();

 private:
  int32_t write_count_ = 0;
  int32_t read_count_ = 0;

  std::mutex rw_count_mtx_;
  std::condition_variable r_cv;
  std::condition_variable w_cv;
};

#endif