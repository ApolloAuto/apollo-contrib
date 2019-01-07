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
 * @file v2x_data.h
 * @brief define V2X data class
 */

#pragma once

#include <stdint.h>

namespace v2x {

class V2xData {
 public:
  V2xData(char* buf, uint32_t len);
  ~V2xData();
  char* get_buf() const;
  uint32_t get_len() const;

 private:
  char* buf_;
  uint32_t len_ = 0;
};
}  // namespace v2x
