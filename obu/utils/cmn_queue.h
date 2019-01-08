/* Copyright 2017 The Apollo Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
=========================================================================*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace v2x {

template <typename DataType>
class CmnQueue {
 public:
  CmnQueue() : max_size_(1) {}
  explicit CmnQueue(uint32_t size) {
    max_size_ = size > 0 ? size : 1;
  }
  ~CmnQueue() = default;

  bool is_empty() const {
    std::lock_guard<std::mutex> lk(queue_mutex_);
    return ptr_queue_.empty();
  }

  void push(DataType data) {
    // std::make_shared<>() may throw std::bad_alloc or any exception thrown by
    // the constructor of DataType. If an exception is thrown, it has no effect.
    try {
      std::shared_ptr<DataType> data_ptr(
          std::make_shared<DataType>(std::move(data)));

      std::lock_guard<std::mutex> lk(queue_mutex_);
      ptr_queue_.push(data_ptr);
      while (ptr_queue_.size() > max_size_) {
        ptr_queue_.pop();
      }
      queue_condition_.notify_one();
    } catch (const std::bad_alloc& ba) {
      std::cout << ba.what() << std::endl;
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
    } catch (...) {
      std::cout << "Exception is thrown in CmnQueue::push." << std::endl;
    }
  }

  std::shared_ptr<DataType> pop() {
    std::unique_lock<std::mutex> lk(queue_mutex_);
    queue_condition_.wait(lk, [this] { return !ptr_queue_.empty(); });
    std::shared_ptr<DataType> ret_ptr = ptr_queue_.front();
    ptr_queue_.pop();
    return ret_ptr;
  }

  std::shared_ptr<DataType> try_pop() {
    std::lock_guard<std::mutex> lk(queue_mutex_);
    if (ptr_queue_.empty()) {
      return std::shared_ptr<DataType>(nullptr);
    }
    std::shared_ptr<DataType> ret_ptr = ptr_queue_.front();
    ptr_queue_.pop();
    return ret_ptr;
  }

  std::shared_ptr<DataType> read() const {
    std::unique_lock<std::mutex> lk(queue_mutex_);
    queue_condition_.wait(lk, [this] { return !ptr_queue_.empty(); });
    return ptr_queue_.front();
  }

  std::shared_ptr<DataType> try_read() const {
    std::lock_guard<std::mutex> lk(queue_mutex_);
    if (ptr_queue_.empty()) {
      return std::shared_ptr<DataType>(nullptr);
    }
    return ptr_queue_.front();
  }

 private:
  uint64_t max_size_;
  mutable std::mutex queue_mutex_;
  std::queue<std::shared_ptr<DataType>> ptr_queue_;
  std::condition_variable queue_condition_;
};

}  // namespace v2x

