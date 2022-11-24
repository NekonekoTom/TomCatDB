#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream> // for test

// Usage:
// Any tasks which will be submitted to the ThreadPool must be packaged
// in a class inherits the ThreadTask and implements the method ExecuteTask().
class ThreadTask {
 public:
  virtual void ExecuteTask() = 0;
};

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

#endif