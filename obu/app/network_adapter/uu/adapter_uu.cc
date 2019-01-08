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
 * @file adapter_uu.cc
 * @brief adaptation module recieves the data from network layer
 *   via uu interface.
 */

#include "adapter_uu.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include "cv2x_app.h"
#include "database/v2x_db.h"
#include "glog/logging.h"
#include "msg_sets/message_set.h"
#include "platform_auth/auth_manager.h"
#include "platform_auth/encrypt_utils.h"

namespace v2x {

AdapterUu::AdapterUu(int fd)
    : OsThread("adapter_uu", OS_THREAD_PRIORITY_NORMAL,
               OS_THREAD_DEFAULT_STACK_SIZE, true),
      recv_socket_(fd) {}

AdapterUu::~AdapterUu(void) {}

// recv will get all data one time
void AdapterUu::RecvData() {
  char* data;
  try {
    data = new char[1024];
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "alloc memory failed";
    return;
  }

  memset(data, 0, 1024);
  uint32_t appid = 0;
  int ret = 0;
  MessageFrame_t* pMsgFrm = NULL;

  V2xDb* data_base_inst = V2xDbSingleton::Instance();
#ifdef COMPUTE_COST
  struct timespec time_start = {0, 0}, time_end = {0, 0};
  clock_gettime(CLOCK_REALTIME, &time_start);
#endif 
  int rlen = recvfrom(recv_socket_, data, 1024, 0, nullptr, nullptr);
#ifdef  COMPUTE_COST
  clock_gettime(CLOCK_REALTIME, &time_end);
  long cost = (time_end.tv_sec - time_start.tv_sec) * 1000000 +
              (time_end.tv_nsec - time_start.tv_nsec) / 1000;

  std::cout << "===========cost time:" << cost << "us===========" << std::endl;
#endif
  if (rlen < 0) {
    LOG(ERROR) << "recvfrom got error: " << rlen << ", error " << strerror(errno);
    delete[] data;
    return;
  }
  LOG(INFO) << "Get data successfully from server, length:" << rlen;

  // decode the message to get the type
  ret = cv2x_message_frame_decode(&pMsgFrm, UPER_CODE_TYPE, data, rlen);
  if (ret != CV2X_SUCCESS) {
    LOG(ERROR) << "decode failed, ret(" << ret << ")";
    delete[] data;
    return;
  }

  if (pMsgFrm->present == MessageFrame_PR_mapFrame) {
    appid = 29;
  } else if (pMsgFrm->present == MessageFrame_PR_spatFrame) {
    appid = 30;
  } else {
    LOG(INFO) << "the message type " << pMsgFrm->present;
    ret = cv2x_message_frame_free(pMsgFrm, 0);
    if (ret != 0) {
      LOG(ERROR) << "free message failed (" << ret << ")";
    }
    delete[] data;
    return;
  }
  ret = cv2x_message_frame_free(pMsgFrm, 0);
  if (ret != 0) {
    LOG(ERROR) << "free message failed (" << ret << ")";
  }
  data_base_inst->SubmitData(data, rlen, appid);

  return;
}

void AdapterUu::Run() {
  BaseAuthManager* auth_manager = BaseAuthManagerSingleton::Instance();
  AUTH_STATUS auth_status = NONE;

  while (1) {
    while (auth_status != AUTHED) {
      usleep(5000);
      auth_status = auth_manager->GetAuthStatus();
    }
    RecvData();
  }
}
}  // namespace v2x
