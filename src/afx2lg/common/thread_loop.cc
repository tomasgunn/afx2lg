// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "common/thread_loop.h"

namespace common {

#ifdef _WIN32
using std::cv_status::cv_status;
#else
using std::cv_status;
#endif

ThreadLoop::ThreadLoop()
    : timeout_(std::chrono::milliseconds(1000 * 60 * 10)),
      is_running_(false) {
}

ThreadLoop::~ThreadLoop() {}

bool ThreadLoop::is_running() const {
  std::lock_guard<std::mutex> lock(lock_);
  return is_running_;
}

bool ThreadLoop::empty() const {
  std::lock_guard<std::mutex> lock(lock_);
  return queue_.empty();
}

void ThreadLoop::set_timeout(const std::chrono::milliseconds& timeout) {
  std::lock_guard<std::mutex> lock(lock_);
  timeout_ = timeout;
}

bool ThreadLoop::Run() {
  {
    std::lock_guard<std::mutex> lock(lock_);
    ASSERT(!is_running_);
    is_running_ = true;
  }

  Task task;
  while (PopTask(&task)) {
    task();
    if (!is_running())
      return true;
  }

  return false;
}

void ThreadLoop::Quit() {
  QueueTask(std::bind(&ThreadLoop::SetQuit, this));
}

void ThreadLoop::QueueTask(const Task& task) {
  {
    std::lock_guard<std::mutex> lock(lock_);
    queue_.push(std::move(task));
  }
  signal_.notify_one();
}

void ThreadLoop::SetQuit() {
  std::lock_guard<std::mutex> lock(lock_);
  is_running_ = false;
}

bool ThreadLoop::PopTask(ThreadLoop::Task* task) {
  std::unique_lock<std::mutex> lock(lock_);
  while (queue_.empty()) {
    cv_status status = signal_.wait_for(lock, timeout_);
    if (status == cv_status::timeout)
      return false;
  }

  ASSERT(lock.owns_lock());
  *task = std::move(queue_.front());
  queue_.pop();

  return true;
}

}  // namespace common
