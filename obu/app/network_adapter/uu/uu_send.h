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
 * @file uu_send.h
 * @brief define Adapter class as a thread
 */

#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include "os_thread.h"

namespace v2x {

class UuSend final : public OsThread {
 public:
  UuSend(std::string src_addr, uint32_t src_port, uint32_t interval);
  virtual ~UuSend();

  void Initialize();

  virtual void Run();
  int GetSocketFd();
 private:
  void TimerExpired();
  bool SendData(char* buf, uint32_t len);
  std::string src_addr_;
  uint32_t src_port_ = 0;
  sockaddr_in send_addr_;
  int send_socket_ = 0;
  uint32_t interval_ = 0;
  int timer_fd_ = 0;
  bool is_cyclical_ = false;
};

}  // namespace v2x
