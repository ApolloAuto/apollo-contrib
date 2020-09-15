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
 * @file traffic_light_thread.cc
 * @brief traffic light thread.
 */

#include "traffic_light_thread.h"

#include "database/v2x_db.h"
#include "glog/logging.h"
#include "msg_sets/message_set.h"
#include "network_adapter/pc5/adapter_pc5.h"
#include "policies/trafficlight_process/v2x_traffic_light.h"
#include "proxy/proxy.h"

namespace v2x {

void TrafficLightThread::Run() {
  bool flag = false;
  char* map_buf = nullptr;
  char* spat_buf = nullptr;
  uint32_t mtu = GetNetworkMtu();
  uint32_t spat_buf_len = mtu;
  uint32_t map_buf_len = mtu;
  MessageSet map_msg;
  MessageSet spat_msg;
  int ret = 0;
  SPAT_t* spat = nullptr;
  MapData_t* map = nullptr;

  V2xProxy* grpc = V2xProxySingleton::Instance();
  V2xDb* data_base_inst = V2xDbSingleton::Instance();

  v2x::Trafficlight tl;
  tl.Init();

  try {
    map_buf = new char[map_buf_len];
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "memory malloc failed for map msg buf";
    return;
  }
  try {
    spat_buf = new char[spat_buf_len];
  } catch (const std::bad_alloc& e) {
    LOG(ERROR) << "memory malloc failed for spat msg buf";
    delete[] map_buf;
    return;
  }

  while (1) {
    spat_buf_len = mtu;
    map_buf_len = mtu;

    memset(map_buf, 0, map_buf_len);
    memset(spat_buf, 0, spat_buf_len);
    // Read the spat message
    flag = data_base_inst->GetData(spat_buf, &spat_buf_len, MSG_TYPE_SPAT);
    if (!flag) {
      LOG(ERROR) << "get the spat msg is null";
      continue;
    }
    LOG(INFO) << "recieved the spat message with " << spat_buf_len;

    ret = spat_msg.CreateSpat();
    if (ret != 0) {
      LOG(ERROR) << "create spat message failed (" << ret << ")";
      continue;
    }

    ret = spat_msg.DecodeSpat(spat_buf, UPER_CODE_TYPE, spat_buf_len);
    if (ret != 0) {
      LOG(ERROR) << "decode spat message failed (" << ret << ")";
      continue;
    }
    spat = spat_msg.GetSpat();
    if (spat == nullptr) {
      LOG(ERROR) << "spat message is nullptr";
      continue;
    }

    // Read the map message
    flag = data_base_inst->GetData(map_buf, &map_buf_len, MSG_TYPE_MAP);
    if (!flag) {
      LOG(ERROR) << "get the map msg is null";
      spat_msg.FreeAll();
      continue;
    }
    LOG(INFO) << "recieved the map message with " << map_buf_len;

    ret = map_msg.CreateMap();
    if (ret != 0) {
      LOG(ERROR) << "create the map message failed (" << ret << ")";
      spat_msg.FreeAll();
      continue;
    }

    ret = map_msg.DecodeMap(map_buf, UPER_CODE_TYPE, map_buf_len);
    if (ret != 0) {
      LOG(ERROR) << "decode the map message failed (" << ret << ")";
      spat_msg.FreeAll();
      continue;
    }
    map = map_msg.GetMap();
    if (map == nullptr) {
      LOG(ERROR) << "map message is nullptr";
      spat_msg.FreeAll();
      continue;
    }

    auto car_status = grpc->GetCarStatus();
    if (car_status == nullptr) {
      spat_msg.FreeAll();
      map_msg.FreeAll();
      continue;
    }

    std::shared_ptr<apollo::v2x::obu::ObuTrafficLight>
        intersection_trafficlight_msg(new apollo::v2x::obu::ObuTrafficLight);
    ret = tl.TrafficLightApp(map, spat, car_status,
                             intersection_trafficlight_msg);
    if (ret != 0) {
      LOG(ERROR) << "get traffic light failed (" << ret << ")";
      spat_msg.FreeAll();
      map_msg.FreeAll();
      continue;
    }

    grpc->SendTrafficLights(*intersection_trafficlight_msg);

    spat_msg.FreeAll();
    map_msg.FreeAll();
  }
  delete[] spat_buf;
  delete[] map_buf;
}

}  // namespace v2x
