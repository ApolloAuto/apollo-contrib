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
 * @file v2x_data.cc
 * @brief implement the v2x data class
 */

#include "v2x_data.h"

namespace v2x {
V2xData::V2xData(char* buf, uint32_t len) : buf_(buf), len_(len) {}

V2xData::~V2xData() {
  if (nullptr != buf_) {
    delete[] buf_;
  }
}

char* V2xData::get_buf() const {
  //  std::cout << "v2x get buf " << (void*)buf_ << std::endl;
  return buf_;
}

uint32_t V2xData::get_len() const {
  return len_;
}

}  // namespace v2x
