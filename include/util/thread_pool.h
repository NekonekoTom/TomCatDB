#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>  // for test
#include "base.h"
#include "raii_lock.h"

// Deprecated
// Usage:
// Any tasks which will be submitted to the ThreadPool must be packaged
// in a class inherits the ThreadTask and implements the method ExecuteTask().
class ThreadTask {
 public:
  virtual void ExecuteTask() = 0;
};

// Deprecated
class ThreadPool {
 public:
  ThreadPool();
  ThreadPool(const int max_task_queue_size, const int max_core_thread_num);
  ~ThreadPool();

  bool Start();
  bool SubmitTask(ThreadTask*);

 private:
  const int kMaxTaskQueueSize;
  const int kMaxCoreThreadNum;

  void InitThreadPool();
  void BackgroundThreadTask();

  std::deque<ThreadTask*> task_queue_;
  std::deque<std::thread*> thread_queue_;
  std::mutex mtx_;
  std::condition_variable cv_;
  std::thread* main_thread;
};

template <typename T>
class ConcurrentQueue {
 public:
  ConcurrentQueue() : lock(mtx_) {}
  ConcurrentQueue(const ConcurrentQueue&) = delete;
  ConcurrentQueue& operator()(const ConcurrentQueue&) = delete;

  ~ConcurrentQueue() = default;

  const typename std::deque<T>::size_type size() const { return data_.size(); }

  bool empty() const { return size() == 0; }

  void Enqueue(const T& t) {
    lock.Lock();
    data_.push_back(t);
    lock.Unlock();
  }

  void Enqueue(T&& t) {
    lock.Lock();
    data_.push_back(t);
    lock.Unlock();
  }

  bool Dequeue(T& t) {
    lock.Lock();
    if (!data_.empty()) {
      t = std::move(data_.front());
      data_.pop_front();
      lock.Unlock();
      return true;
    }
    lock.Unlock();
    return false;
  }

 private:
  std::deque<T> data_;
  std::mutex mtx_;
  RAIILock lock;
};

class TCThreadPool {
 public:
  TCThreadPool();
  TCThreadPool(const int default_core_thread_num,
               const int max_task_queue_size);

  ~TCThreadPool();

  template <typename F, typename... Args>
  auto SubmitTask(F&& f, Args&&... args) -> std::future<
      decltype(std::forward<F>(f)(std::forward<Args>(args)...))> {

    typedef decltype(std::forward<F>(f)(std::forward<Args>(args)...)) func_type;

    std::function<func_type()> task_func =
        std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    auto task_ptr =
        std::make_shared<std::packaged_task<func_type()>>(task_func);
    // Wrong. The task object will be deconstructed when SubmitTask() exits.
    // std::packaged_task<func_type> task(task_func);

    // Wrap std::package_task into void function
    std::function<void()> wrapped_func = [task_ptr]() {
      (*task_ptr)();
      // Equivalent to: task_func();
    };

    // Wrong. packaged_task forbids copy.
    // std::function<void()> wrapped_func = [task]() {...}
    // Wrong. packaged_task will be deconstructed.
    // std::function<void()> wrapped_func = [&task]() {...}

    task_queue_.Enqueue(wrapped_func);

    cv_.notify_one();

    return task_ptr->get_future();
  }

  // Start the thread pool. Core threads are constructed and stored in queue.
  void Start();

  // Stop the thread pool. Resources will be reclaimed after all thread tasks
  // are done.
  void Shutdown();

 private:
  const int kDefaultCoreThreadNum;
  const int kMaxTaskQueueSize;

  void BackgroundThreadTask();

  ConcurrentQueue<std::function<void()>> task_queue_;
  std::deque<std::thread> thread_queue_;
  std::mutex mtx_;
  std::condition_variable cv_;
  std::atomic<bool> is_thread_pool_running_;
};

#endif