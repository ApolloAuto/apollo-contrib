/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file os_thread.h
 * @brief define thread class
 */

#pragma once

#include <pthread.h>
#include <string>

namespace v2x {

const int os_wait_forever = -1;
const int os_no_wait = 0;
enum Priority {
  OS_THREAD_PRIORITY_IDLE = 10,
  OS_THREAD_PRIORITY_LOWEST = 30,
  OS_THREAD_PRIORITY_BELOW_NORMAL = 70,
  OS_THREAD_PRIORITY_NORMAL = 90,
  OS_THREAD_PRIORITY_ABOVE_NORMAL = 92,
  OS_THREAD_PRIORITY_HIGHEST = 95,
  OS_THREAD_PRIORITY_TIME_CRITICAL = 99,
};

enum {
  OS_THREAD_DEFAULT_STACK_SIZE = 512 * 1024,  // 512k
};

class OsThread {
 public:
  /**
   * Create a thread.
   */
  OsThread(const char* pName, int prio = OS_THREAD_PRIORITY_NORMAL,
           int stacksize = OS_THREAD_DEFAULT_STACK_SIZE, bool joinable = false);
  virtual ~OsThread() noexcept;

  /**
   * It will execute the Run method within this thread context.
   */
  void Start();

  /**
   * Wait for the thread to exit.
   *
   * @param timeout The timeout value in ms. Default zero means forever
   */
  int Join(int timeout = os_wait_forever);

  /**
   * Get the thread name
   */
  const char* GetName() const;

 protected:
  /**
   * The run method should be implemented by derived class.
   *
   * When this finishes, the thread itself will be destroyed
   * if it is not joinable.
   */
  virtual void Run() = 0;

 private:
  OsThread(const OsThread&);
  OsThread& operator=(const OsThread&);

  static void* ThreadProc(void* arg);

  std::string name_;
  int prio_;
  int stack_size_;
  bool joinable_;

  pthread_t thread_id_;
};
}  // namespace v2x
