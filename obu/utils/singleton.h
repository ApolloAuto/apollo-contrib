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
 * @file singleton.h
 * @brief define singleton class
 */

#pragma once
#include <mutex>

namespace v2x {
template <typename Singleton>
class SingletonService {
 public:
  // Instance() returns the singleton object .
  static Singleton* Instance(void);
  // InstallInstance() install the singleton object.
  static void InstallInstance(Singleton* pinstance);
  // UninstallInstance() uninstall the singleton object.
  static Singleton* UninstallInstance();
  // Check if the instance is installed or not
  static bool IsInstalled(void);

 private:
  static Singleton* instance_;
  static std::mutex mutex_;
};
template <typename Singleton>
Singleton* SingletonService<Singleton>::instance_ = NULL;
template <typename Singleton>
std::mutex SingletonService<Singleton>::mutex_;
template <typename Singleton>
inline Singleton* SingletonService<Singleton>::Instance(void) {
  if (instance_ != NULL) {
    return instance_;
  }

  return nullptr;
}

template <typename Singleton>
inline void SingletonService<Singleton>::InstallInstance(
    Singleton* pinstance) {
  std::lock_guard<std::mutex> guard(mutex_);
  instance_ = pinstance;
}

template <typename Singleton>
inline Singleton* SingletonService<Singleton>::UninstallInstance() {
  std::lock_guard<std::mutex> guard(mutex_);
  Singleton* pinstance = instance_;
  instance_ = nullptr;
  return pinstance;
}

template <typename Singleton>
inline bool SingletonService<Singleton>::IsInstalled(void) {
  return (instance_ != nullptr);
}
}  // namespace v2x
