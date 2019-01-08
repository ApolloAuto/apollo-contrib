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
 * @file v2x_table.cc
 * @brief define V2X Table class
 */

#include "v2x_table.h"
#include <string.h>
#include <iostream>
#include "glog/logging.h"

namespace v2x {

V2xTable::V2xTable(bool notify_flag, uint32_t entry_num)
    : notify_flag_(notify_flag), max_entry_num_(entry_num) {}
V2xTable::~V2xTable() {}

bool V2xTable::IsEmpty() {
  std::lock_guard<std::mutex> guard(table_mutex_);
  return table_.empty();
}

int V2xTable::TableSize() {
  return table_.size();
}

void V2xTable::AddEntry(V2xData& data) {
  std::lock_guard<std::mutex> guard(table_mutex_);
  if (TableSize() + 1 > max_entry_num_) {
    V2xData* entry = table_.front();
    table_.pop_front();
    delete entry;
  }
  table_.push_front(&data);
  if (notify_flag_) {
    refresh_ = true;
    table_condition_.notify_one();
  }
}

V2xData* V2xTable::GetLatestEntry() {
  V2xData* entry = nullptr;
  if (notify_flag_) {
    std::unique_lock<std::mutex> guard(table_mutex_);
    table_condition_.wait(guard, [this] { return refresh_; });
    entry = table_.front();
    refresh_ = false;
  } else {
    std::lock_guard<std::mutex> guard(table_mutex_);
    if (table_.empty()) return nullptr;
    entry = table_.front();
  }
  return entry;
}

bool V2xTable::AddEntryWithCompare(V2xData& data) {
  std::lock_guard<std::mutex> guard(table_mutex_);

  for (auto& v2x_data : table_) {
    if (v2x_data->get_len() == data.get_len()) {
      if (memcmp(v2x_data->get_buf(), data.get_buf(), data.get_len()) == 0) {
        LOG(INFO) << "buf content is same, ignore this message with len "
                  << data.get_len();
        return false;
      }
    }
  }
  // check the table size, if > max_entry_num, delete one
  if (TableSize() + 1 > max_entry_num_) {
    V2xData* entry = table_.front();
    if (entry != nullptr) {
      table_.pop_front();
      delete entry;
    }
  }
  table_.push_front(&data);
  if (notify_flag_) {
    refresh_ = true;
    table_condition_.notify_one();
  }
  return true;
}
}  // namespace v2x
