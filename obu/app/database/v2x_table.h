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
 * @file v2x_table.h
 * @brief define V2X table class
 */

#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include "v2x_data.h"

namespace v2x {

class V2xTable {
 public:
  V2xTable(bool notify_flag, uint32_t entry_num);
  ~V2xTable();

  bool IsEmpty();
  int TableSize();
  void AddEntry(V2xData& data);
  V2xData* GetLatestEntry();
  bool AddEntryWithCompare(V2xData& data);

 private:
  bool notify_flag_ = false;
  uint32_t max_entry_num_ = 0;
  std::mutex table_mutex_;
  std::list<V2xData*> table_;
  bool refresh_ = false;
  std::condition_variable table_condition_;
};
}  // namespace v2x
