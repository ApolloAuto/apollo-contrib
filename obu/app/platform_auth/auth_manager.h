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
 * @brief This library provides register and authentication to platform
 */

#ifndef APP_PLATFORM_AUTH_AUTH_MANAGER_H
#define APP_PLATFORM_AUTH_AUTH_MANAGER_H

#include <iostream>
#include <memory>
#include <string>
#include "auth_manager.h"
#include "auth_para.h"
#include "device_info.h"
#include "os_thread.h"
#include "singleton.h"

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {

// vendor code
enum PLATFORM_VENDOR { CMCC_V2X };

// authentication status
enum AUTH_STATUS {
  NONE = 0,
  REGISTED,
  AUTHED,
  INVALID_ALL = 100,
  INVALID_KEY,
  INVALID_TOKEN
};

/**
 * @class BaseAuthManager
 *
 * @brief base class of authentication manager
 */
class BaseAuthManager : public OsThread {
 public:
  /**
   * @brief constructor
   */
  BaseAuthManager()
      : OsThread("BaseAuthManager", OS_THREAD_PRIORITY_NORMAL,
                 OS_THREAD_DEFAULT_STACK_SIZE, true) {
                 }
  /**
   * @brief destructor
   */
  virtual ~BaseAuthManager() {}

  /**
   * @brief create an authentication manager
   * @param vendor_code: the code standing for platform vendor
   *        auth_para: reference to an AuthPara object
   *        device_info: reference to a DeviceInfo object
   * @return pointer to an BaseAuthManager object
   */
  static BaseAuthManager *Create(int vendor_code, AuthPara &auth_para,
                             DeviceInfo &device_info);

  /**
   * @brief send request to platform to register the device
   * @param void
   * @return true if success, otherwise false returned
   */
  virtual bool DeviceRegister() = 0;

  /**
   * @brief send request to platform to authenticate the device
   * @param void
   * @return true if success, otherwise false returned
   */
  virtual bool DeviceAuth() = 0;

  /**
   * @brief set authentication status
   * @param status: authentication status
   * @return void
   */
  virtual void SetAuthStatus(AUTH_STATUS status) = 0;

  /**
   * @brief get the authentication status
   * @param void
   * @return authentication status
   */
  virtual AUTH_STATUS GetAuthStatus() = 0;

  /**
   * @brief update authentication status
   * @param void
   * @return void
   */
  virtual void UpdateAuthStatus() = 0;

  /**
   * @brief get the key to encrypt bsm message
   * @param void
   * @return the key
   */
  virtual std::string GetKey() = 0;

  /**
   * @brief get the valid time of the key
   * @param void
   * @return the valid time
   */
  virtual std::string GetKeyValidTime() = 0;

  /**
   * @brief get the token to fill bsm message
   * @param void
   * @return the token
   */
  virtual std::string GetToken() = 0;

  /**
   * @brief get the valid time of the token
   * @param void
   * @return the valid time
   */
  virtual std::string GetTokenValidTime() = 0;

  /**
   * @brief get id of the device
   * @param void
   * @return id of the device
   */
  virtual std::string GetDeviceId() = 0;

  /**
   * @brief process authentication
   * @param void
   * @return void
   */
  virtual void AuthProc() = 0;

  /**
   * @brief run function
   * @param void
   * @return void
   */
  virtual void Run() {
    AuthProc();
  }
};

/**
 * @class AuthManager
 *
 * @brief auth manager for platform
 */
class AuthManager : public BaseAuthManager {
 public:
  /**
   * @brief constructor
   */
  AuthManager(AuthPara &auth_para, DeviceInfo &device_info);

  /**
   * @brief destructor
   */
  ~AuthManager() {}

  /**
   * @brief send request to platform to register the device
   * @param void
   * @return true if success, otherwise false returned
   */
  bool DeviceRegister();

  /**
   * @brief send request to platform to authenticate the device
   * @param void
   * @return true if success, otherwise false returned
   */
  bool DeviceAuth();

  /**
   * @brief set authentication status
   * @param status: authentication status
   * @return void
   */
  void SetAuthStatus(AUTH_STATUS status);

  /**
   * @brief get the authentication status
   * @param void
   * @return AUTH_STATUS
   */
  AUTH_STATUS GetAuthStatus();

  /**
   * @brief update authentication status
   * @param void
   * @return void
   */
  void UpdateAuthStatus();

  /**
   * @brief get the key to encrypt bsm message
   * @param void
   * @return the key
   */
  std::string GetKey() {
    return auth_para_.GetKey();
  }

  /**
   * @brief get the valid time of the key
   * @param void
   * @return the valid time
   */
  std::string GetKeyValidTime() {
    return auth_para_.GetKeyValidTime();
  }

  /**
   * @brief get the token to fill bsm message
   * @param void
   * @return the token
   */
  std::string GetToken() {
    return auth_para_.GetToken();
  }

  /**
   * @brief get the valid time of the token
   * @param void
   * @return the valid time
   */
  std::string GetTokenValidTime() {
    return auth_para_.GetTokenValidTime();
  }

  /**
   * @brief get id of the device
   * @param void
   * @return id of the device
   */
  std::string GetDeviceId() {
    return device_info_.GetDeviceId();
  }

  /**
   * @brief process authentication
   * @param void
   * @return void
   */
  virtual void AuthProc();

 private:
  /**
   * @brief constructor
   */
  AuthManager(const AuthManager &auth_manager);

  /**
   * @brief operator=
   */
  void operator=(const AuthManager &auth_manager);
  /**
   * @brief get authentication type
   * @param void
   * @return authentication type: 0 is to request for token and key,
   * 1 is to request for token, and 2 is request for key
   */
  int AuthType();

  /**
   * @brief parse the authentication response data from platform
   * @param response: response data
   * @return ture if success
   */
  bool ParseResponseBody(std::string response);

  /**
   * @brief get current time
   * @param void
   * @return current time in "YYYYMMDDhhmmsssss"
   */
  std::string CurrentTime();

  /**
   * @brief get body of request to platform
   * @param void
   * @return body of request
   */
  std::string RequestBody();

  /**
   * @brief parsing time in string
   * @param time: time in "YYYYMMDDhhmmsssss"
   * @return time in sturct timeval
   */
  struct timeval ParseStringTime(std::string time);

  /**
   * @brief get seconds to valid time
   * @param valid_time: valid time in "YYYYMMDDhhmmsssss"
   * @return seconds
   */
  time_t SecFromValidTime(std::string valid_time);

  /**
   * @brief check if over the valid time
   * @param valid_time: valid time in "YYYYMMDDhhmmsssss"
   * @return true if not over the valid time
   */
  bool CheckValidTime(std::string valid_time);

  /**
   * @brief check if the key is in the valid time
   * @param void
   * @return true if in the valid time
   */
  bool CheckKeyValid();

  /**
   * @brief check if the token is in the valid time
   * @param void
   * @return true if in the valid time
   */
  bool CheckTokenValid();

  /**
   * @brief authenticate the device according to the authentication status
   * @param void
   * @return void
   */
  void DoDeviceAuth();

  static const int VALID_TIME_INTERVAL = 30;
  AuthPara &auth_para_;
  DeviceInfo &device_info_;
  AUTH_STATUS auth_status_;
};
typedef SingletonService<BaseAuthManager> BaseAuthManagerSingleton;
}  // namespace v2x

#endif /* APP_PLATFORM_AUTH_AUTH_MANAGER_H */
