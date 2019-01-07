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
 * @file adapter_pc5.cc
 * @brief adaptation module recieves the data from pc5 interface.
 */

#include "adapter_pc5.h"
#include <iostream>
#include "cv2x_app.h"
#include "database/v2x_db.h"
#include "glog/logging.h"
#include "security.h"

namespace v2x {

eMsgType GetMsgType(uint32_t appid) {
  return get_msg_type(appid);
}

uint32_t GetNetworkMtu() {
  return get_mtu();
}

char* Adapter::MallocStr(uint32_t buf_size) {
  try {
    char* buf = new char[buf_size];
    memset(buf, 0, buf_size);
    return buf;
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "memory malloc failed for buf";
    return nullptr;
  }
}

int Adapter::RecvPackage(char* buf, uint32_t len, tDSMRxParm* dsmRxParm) {
  int cnt = 0;

  cnt = v2x_system_read(buf, len, dsmRxParm);
  if (cnt <= 0) {
    LOG(ERROR) << "recv wrong(" << cnt << ")";
    return -1;
  }
  return cnt;
}

void Adapter::ProcessUnsecured() {
  char* buf = nullptr;
  uint32_t len = GetNetworkMtu();
  int cnt = 0;
  tDSMRxParm dsmRxParm = {0};
  V2xDb* data_base_inst = V2xDbSingleton::Instance();

  while (1) {
    buf = MallocStr(len);
    if (buf == nullptr) {
      continue;
    }

    cnt = RecvPackage(buf, len, &dsmRxParm);
    if (cnt > 0) {
      data_base_inst->SubmitData(buf, cnt, dsmRxParm.appId);
    } else {
      delete[] buf;
    }
  }
}

void Adapter::ProcessSecured() {
  uint32_t len = GetNetworkMtu();
  char* buf = nullptr;
  int cnt = 0;
  tDSMRxParm dsmRxParm = {0};
  Security security(root_ca_, self_ca_);
  char* data = nullptr;
  uint32_t data_len = 0;
  V2xDb* data_base_inst = V2xDbSingleton::Instance();

  if (security.Init() != 0) {
    LOG(ERROR) << "Security init failed";
    return;
  }
  try {
    buf = new char[len];
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "memory malloc failed for buf";
    return;
  }
  while (1) {
    memset(buf, 0, len);

    data = MallocStr(len);
    if (data == nullptr) {
      continue;
    }

    cnt = RecvPackage(buf, len, &dsmRxParm);
    if (cnt <= 0) {
      delete[] data;
      continue;
    }

    if (security.VerifyAndStrip((uint8_t*)buf, cnt, (uint8_t*)data,
                                &data_len) != 0) {
      LOG(ERROR) << "Security verify failed";
      delete[] data;
      continue;
    }
    data_base_inst->SubmitData(data, data_len, dsmRxParm.appId);
  }
}

void Adapter::Run() {
  int32_t ret = 0;

  ret = v2x_system_init();
  if (ret != 0) {
    LOG(ERROR) << "V2X system init failed";
    return;
  }

  if (!security_flag_) {
    return ProcessUnsecured();
  } else {
    return ProcessSecured();
  }
}

}  // namespace v2x

