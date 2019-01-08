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
 * @file adapter_uu.h
 * @brief define Adapter class as a thread
 */

#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include "os_thread.h"

namespace v2x {

class AdapterUu final : public OsThread {
 public:
  AdapterUu(int fd);
  virtual ~AdapterUu();
  void Initialize();
  virtual void Run();

 private:
  void RecvData();
  std::string dst_addr_;
  uint32_t dst_port_ = 0;
  int recv_socket_ = 0;
  sockaddr_in recv_addr_;
};

}  // namespace v2x
