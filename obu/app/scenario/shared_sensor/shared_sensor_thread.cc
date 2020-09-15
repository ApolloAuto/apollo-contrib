/******************************************************************************
 * Copyright 2020 The Apollo Authors. All Rights Reserved.
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
 * @file shared_sensor_thread.cc
 * @brief shared sensor thread.
 */

#include "shared_sensor_thread.h"

#include <cstring>

#include "apollo_ssm_decoder.h"
#include "database/v2x_db.h"
#include "glog/logging.h"
#include "msg_sets/message_set.h"
#include "network_adapter/pc5/adapter_pc5.h"
#include "proxy/proxy.h"

namespace v2x {

void SharedSensorThread::Run() {
  bool flag = false;
  uint32_t mtu = GetNetworkMtu();
  uint32_t ssm_buf_len = mtu;
  char* ssm_buf = nullptr;
  int ret = 0;

  V2xProxy* grpc = V2xProxySingleton::Instance();
  V2xDb* data_base_inst = V2xDbSingleton::Instance();

  try {
    ssm_buf = new char[ssm_buf_len];
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "memory malloc failed for ssm msg buf";
    return;
  }
  auto v2x_obstacles = std::make_shared<apollo::v2x::V2XObstacles>();
  if (!v2x_obstacles) {
    LOG(ERROR) << "failed to create obstacles";
    return;
  }
  while (1) {
    v2x_obstacles->Clear();
    ssm_buf_len = mtu;
    std::memset(ssm_buf, 0, ssm_buf_len);
    // Read the ssm message
    flag = data_base_inst->GetData(ssm_buf, &ssm_buf_len, MSG_TYPE_SSM);
    if (!flag) {
      LOG(ERROR) << "get the ssm msg is null";
      continue;
    }
    LOG(INFO) << "recieved the ssm message with " << ssm_buf_len;
    std::string proto_str;
    if (::apollo::v2x::obu::conv::DecodeSSM(ssm_buf, ssm_buf_len, &proto_str)) {
      LOG(ERROR) << "Failed to decode SSM";
      continue;
    }
    if (!v2x_obstacles->ParsePartialFromString(proto_str)) {
      LOG(ERROR) << "Failed to call ParsePartialFromString";
      continue;
    }
    grpc->SendObstacles(*v2x_obstacles);
  }
  delete[] ssm_buf;
}

}  // namespace v2x
