// Copyright (c) 2012, Tomas Gunnarsson
// All rights reserved.

#include "gtest/gtest.h"

#include "common/thread_loop.h"

namespace common {
namespace {
template<typename T>
void Assign(T* t, const T& val) { *t = val; }
}  // namespace

TEST(ThreadLoop, Simple) {
  ThreadLoop loop;
  loop.Quit();
  EXPECT_TRUE(loop.Run());
}

TEST(ThreadLoop, Timeout) {
  ThreadLoop loop;
  loop.set_timeout(std::chrono::milliseconds(1));
  EXPECT_FALSE(loop.Run());
}

TEST(ThreadLoop, QuitFromWorker) {
  ThreadLoop loop;
  std::thread worker(std::bind(&ThreadLoop::Quit, &loop));
  EXPECT_TRUE(loop.Run());
  worker.join();
}

TEST(ThreadLoop, QueueTask) {
  bool task_ran = false;
  ThreadLoop loop;
  loop.QueueTask(std::bind(&Assign<bool>, &task_ran, true));
  loop.Quit();
  EXPECT_TRUE(loop.Run());
  EXPECT_TRUE(task_ran);
}

TEST(ThreadLoop, QueueTaskFromTask) {
  ThreadLoop loop;
  loop.QueueTask(std::bind(&ThreadLoop::Quit, &loop));
  EXPECT_TRUE(loop.Run());
}

}  // namespace common
