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
 * @file traffic_light_thread.h
 * @brief define traffic light class as a thread
 */

#pragma once

#include "os_thread.h"

namespace v2x {

class TrafficLightThread final : public OsThread {
 public:
  TrafficLightThread()
      : OsThread("tl_user_thrd", OS_THREAD_PRIORITY_NORMAL,
                 OS_THREAD_DEFAULT_STACK_SIZE, true){};
  virtual ~TrafficLightThread() {};
  virtual void Run();

 private:
};
}  // namespace v2x
