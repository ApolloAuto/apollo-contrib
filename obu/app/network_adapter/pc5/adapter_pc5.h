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
 * @file adapter_pc5.h
 * @brief define Adapter class as a thread
 */

#pragma once

#include "cv2x_app.h"
#include "os_thread.h"

namespace v2x {

eMsgType GetMsgType(uint32_t appid);
uint32_t GetNetworkMtu();

class Adapter final : public OsThread {
 public:
  Adapter(bool security_flag, std::string root_ca, std::string self_ca)
      : OsThread("adapter", OS_THREAD_PRIORITY_NORMAL,
                 OS_THREAD_DEFAULT_STACK_SIZE, true),
        security_flag_(security_flag),
        root_ca_(root_ca),
        self_ca_(self_ca){};
  virtual ~Adapter() = default;
  virtual void Run();

  //  eMsgType GetMsgType(uint32_t appid);
  //  uint32_t GetNetworkMtu();
 private:
  char* MallocStr(uint32_t buf_size);
  int RecvPackage(char* buf, uint32_t len, tDSMRxParm* dsmRxParm);
  void ProcessSecured();
  void ProcessUnsecured();
  bool security_flag_ = false;
  std::string root_ca_;
  std::string self_ca_;
};
}  // namespace v2x
