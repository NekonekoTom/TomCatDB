#include "thread_pool.h"

ThreadPool::ThreadPool() : kMaxTaskQueueSize(0), kMaxCoreThreadNum(0) {}

ThreadPool::ThreadPool(const int max_task_queue_size,
                       const int max_core_thread_num)
    : kMaxTaskQueueSize(max_task_queue_size),
      kMaxCoreThreadNum(max_core_thread_num) {}

bool ThreadPool::Start() {
  main_thread = new std::thread(&ThreadPool::InitThreadPool, this);
  main_thread->detach();
  return main_thread != nullptr;
}

bool ThreadPool::SubmitTask(ThreadTask* t_task) {
  if (task_queue_.size() >= kMaxTaskQueueSize) {
    return false;
  } else {
    std::unique_lock<std::mutex> lock(mtx_);
    // if (thread_queue_.size())

    task_queue_.push_back(t_task);

    cv_.notify_all();
  }
  return true;
}

ThreadPool::~ThreadPool() {}

void ThreadPool::InitThreadPool() {
  std::unique_lock<std::mutex> lock(mtx_);
  while (task_queue_.empty()) {
    cv_.wait(lock);
  }
  // ThreadTask* first_task = task_queue_.front();
  // task_queue_.pop_front();

  // if (thread_queue_.size() < kMaxCoreThreadNum) {
  while (thread_queue_.size() < kMaxCoreThreadNum) {
    std::thread* new_thread =
        new std::thread(&ThreadPool::BackgroundThreadTask, this);
    new_thread->detach();

    thread_queue_.push_back(new_thread);
  }

  cv_.notify_all();
}

void ThreadPool::BackgroundThreadTask() {
  ThreadTask* first_task = nullptr;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mtx_);
      while (task_queue_.empty()) {
        cv_.wait(lock);
      }
      first_task = task_queue_.front();
      task_queue_.pop_front();
    }

    if (first_task != nullptr) {
      first_task->ExecuteTask();
    }
    // cv_.notify_all();
  }
}

TCThreadPool::TCThreadPool() : TCThreadPool(0, 0) {}

TCThreadPool::TCThreadPool(const int default_core_thread_num,
                           const int max_task_queue_size)
    : kDefaultCoreThreadNum(default_core_thread_num),
      kMaxTaskQueueSize(max_task_queue_size),
      is_thread_pool_running_(false) {}

TCThreadPool::~TCThreadPool() {
  Shutdown();
}

void TCThreadPool::Start() {
  std::unique_lock<std::mutex> lock(mtx_);

  // Set flag
  is_thread_pool_running_.store(true);

  while (thread_queue_.size() < kDefaultCoreThreadNum) {
    thread_queue_.emplace_back(
        std::thread(&TCThreadPool::BackgroundThreadTask, this));
    thread_queue_.back().detach();
  }

  cv_.notify_all();
}

void TCThreadPool::Shutdown() {
  // Reset flag
  is_thread_pool_running_.store(false);

  // Wake up all threads
  cv_.notify_all();
}

void TCThreadPool::BackgroundThreadTask() {
  while (is_thread_pool_running_.load()) {
    std::unique_lock<std::mutex> lock(mtx_);
    while (is_thread_pool_running_.load() && task_queue_.empty()) {
      cv_.wait(lock);
    }

    std::function<void()> task;

    if (task_queue_.Dequeue(task)) {
      task();
    }
  }
}