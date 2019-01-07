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
 * @file v2x_db.cc
 * @brief implement V2X database class
 */

#include "v2x_db.h"
#include <cstring>
#include <iostream>
#include "glog/logging.h"
#include "network_adapter/pc5/adapter_pc5.h"
#include "v2x_data.h"
#include "v2x_table.h"

namespace v2x {

void V2xDb::V2xDbInit() {
  spat_ = new V2xTable(true, 5);
  map_ = new V2xTable(false, 5);
}

void V2xDb::SubmitData(char* buf, uint32_t len, uint32_t appid) {
  V2xData* data = nullptr;
  if (buf == nullptr) {
    return;
  }
  // map message
  if (MSG_TYPE_MAP == GetMsgType(appid)) {
#if 0
    {

        // the code will show the content of buf
        int i = 0;
        printf("recieved the map msg with len %d\n", len);
        for (i = 0; i < len / 16; i++) {
            for (int j = 0; j < 16; j++) {
                printf("%02x", *(uint8_t*)((uint8_t*)buf + i * 16 + j));
            }

            printf("\n");
        }

        for (int j = 0; j < len % 16; j++) {
            printf("%02x", *(uint8_t*)((uint8_t*)buf + i * 16 + j));
        }

        printf("\n");
    }
#endif
    data = new V2xData(buf, len);
    if (!map_->AddEntryWithCompare(*data)) {
      delete data;
    }
    LOG(INFO) << "submit the MAP message to table";
  } else if (MSG_TYPE_SPAT == GetMsgType(appid)) {  // spat message
    data = new V2xData(buf, len);
    if (!spat_->AddEntryWithCompare(*data)) {
       delete data;
    } 
    LOG(INFO) << "submit the SPAT message to table";
  } else {
    delete[] buf;
    LOG(INFO) << "appid is wrong, appid " << appid;
  }
}

bool V2xDb::GetData(char* buf, uint32_t* len, eMsgType type) {
  V2xData* data = nullptr;
  if (MSG_TYPE_MAP == type) {
    data = map_->GetLatestEntry();
  } else if (MSG_TYPE_SPAT == type) {
    data = spat_->GetLatestEntry();
  } else {
    // delete [] buf;
    LOG(INFO) << "type is wrong, appid " << type;
    return false;
  }
  if (data != nullptr) {
    memcpy(buf, data->get_buf(), data->get_len());
    *len = data->get_len();
    return true;
  }
  return false;
}
}  // namespace v2x
