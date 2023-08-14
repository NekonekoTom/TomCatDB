#include "lock_util.h"

void ReadWriteLock::ReadLock() {
  std::unique_lock<std::mutex> lock(rw_count_mtx_);
  while (write_count_ != 0) {
    r_cv.wait(lock);
  }
  ++read_count_;
}

void ReadWriteLock::ReadUnlock() {
  std::unique_lock<std::mutex> lock(rw_count_mtx_);
  --read_count_;
  if (read_count_ == 0 && write_count_ > 0) {
    w_cv.notify_one();
  }
}

void ReadWriteLock::WriteLock(WakeUpOption option) {
  std::unique_lock<std::mutex> lock(rw_count_mtx_);
  ++write_count_;

  switch (option) {
    case AllowSpuriousWakeUp:
      if (write_count_ > 1 || read_count_ != 0) {
        w_cv.wait(lock);
      }
      break;

    // C++ 11 syntactic sugar. This function always protects the thread from
    // `spurious wakeup`
    // w_cv.wait(lock, [this]() -> bool {
    //   return this->write_count_ <= 1 && this->read_count_ == 0;
    // });
    case DisallowSpuriousWakeUp:
      w_cv.wait(lock, [this]() -> bool {
        return this->write_count_ <= 1 && this->read_count_ == 0;
      });
      break;

    default:
      break;
  }
}

void ReadWriteLock::WriteUnlock() {
  std::unique_lock<std::mutex> lock(rw_count_mtx_);
  --write_count_;
  if (write_count_ == 0) {
    r_cv.notify_all();  // Notify all readers
  } else {
    w_cv.notify_one();  // Notify one writer
  }
}