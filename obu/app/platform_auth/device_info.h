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
 * @file
 *
 * @brief This library provides device information.
 */

#pragma once
#include <string>

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {
/**
 * @class DeviceInfo
 *
 * @brief information about the device
 */
class DeviceInfo {
 public:
  /**
   * @brief constructor
   */
  DeviceInfo(const std::string &device_id, const std::string &mac,
             const std::string phone_num) {
    device_id_ = device_id;
    mac_ = mac;
    phone_num_ = phone_num;
  };

  /**
   * @brief destructor
   */
  ~DeviceInfo(){};

  /**
   * @brief get id of the device
   * @param void
   * @return id of the device
   */
  std::string GetDeviceId() {
    return device_id_;
  }

  /**
   * @brief get mac address of the device
   * @param void
   * @return mac address of the device
   */
  std::string GetMac() {
    return mac_;
  }

  /**
   * @brief get phone number of the device
   * @param void
   * @return phone number of the device
   */
  std::string GetPhoneNum() {
    return phone_num_;
  }

 private:
  /**
   * @brief constructor
   */
  DeviceInfo(const DeviceInfo &device_info);

  /**
   * @brief operator=
   */
  void operator=(const DeviceInfo &device_info);

  std::string device_id_;
  std::string mac_;
  std::string phone_num_;
};
}  // namespace v2x
