// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#pragma once
#ifndef COMMON_THREAD_LOOP_H_
#define COMMON_THREAD_LOOP_H_

#include "common_types.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace base {

class ThreadLoop {
 public:
  typedef std::function<void()> Task;

  ThreadLoop();
  ~ThreadLoop();

  bool is_running() const;
  bool empty() const;
  void set_timeout(const std::chrono::milliseconds& timeout);

  // Returns true if the loop ended via Quit();
  // False if a timeout occurred.
  bool Run();

  void Quit();

  void QueueTask(const Task& task);

 private:
  void SetQuit();
  bool PopTask(Task* task);

  std::condition_variable signal_;
  std::chrono::milliseconds timeout_;
  mutable std::mutex lock_;
  std::queue<Task> queue_;
  bool is_running_;
};

typedef std::shared_ptr<ThreadLoop> SharedThreadLoop;

}  // namespace base

#endif  // COMMON_THREAD_LOOP_H_
