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
 * @file v2x_db.h
 * @brief define V2X database class
 */

#pragma once

#include "cv2x_app.h"
#include "singleton.h"
#include "v2x_table.h"

namespace v2x {

class V2xDb {
 public:
  // const static uint32_t map_appid = 29;
  // const static uint32_t spat_appid = 30;
  // const static uint32_t g_max_buf_max = 1000;
  V2xDb() = default;
  ~V2xDb() {
    delete spat_;
    delete map_;
    delete ssm_;
  };
  void V2xDbInit();
  void SubmitData(char* buf, uint32_t len, uint32_t appid);
  bool GetData(char* buf, uint32_t* len, eMsgType type);

 private:
  V2xTable* spat_ = nullptr;
  V2xTable* map_ = nullptr;
  V2xTable* ssm_ = nullptr;
};
typedef SingletonService<V2xDb> V2xDbSingleton;
}  // namespace v2x
