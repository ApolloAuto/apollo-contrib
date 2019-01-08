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
 * @brief This library provides authentication parameters.
 */

#ifndef AUTH_PARA_H_
#define AUTH_PARA_H_

#include <memory>
#include <string>

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {

/**
 * @class AuthPara
 *
 * @brief authentication parameter
 */
class AuthPara {
 public:
  /**
   * @brief constructor
   */
  AuthPara(std::string auth_url)
      : auth_url_(auth_url),
        key_(""),
        token_(""),
        key_valid_time_(""),
        token_valid_time_("") {}

  /**
   * @brief destructor
   */
  ~AuthPara() {}

//   /**
//    * @brief create shared pointer containing an AuhtPara object
//    * @param auth_url: url of the authentication platform
//    * @return shared pointer containing an AuhtPara object
//    */
//   static std::shared_ptr<AuthPara> Create(std::string auth_url) {
//     return std::shared_ptr<AuthPara>(new AuthPara(auth_url));
//   }

  /**
   * @brief get url of the authentication platform
   * @param void
   * @return url of the authentication platform
   */
  std::string GetAuthUrl() {
    return auth_url_;
  }

  /**
   * @brief get the key to encrypt bsm message
   * @param void
   * @return the key
   */
  std::string GetKey() {
    return key_;
  }

  /**
   * @brief get the token to fill bsm message
   * @param void
   * @return the token
   */
  std::string GetToken() {
    return token_;
  }

  /**
   * @brief get the valid time of the key
   * @param void
   * @return the valid time
   */
  std::string GetKeyValidTime() {
    return key_valid_time_;
  }

  /**
   * @brief get the valid time of the token
   * @param void
   * @return the valid time
   */
  std::string GetTokenValidTime() {
    return token_valid_time_;
  }

  /**
   * @brief set the key to encrypt bsm message
   * @param key: the key
   * @return void
   */
  void SetKey(std::string key) {
    key_ = key;
  }

  /**
   * @brief set the key filling in bsm message
   * @param token: the token
   * @return void
   */
  void SetToken(std::string token) {
    token_ = token;
  }

  /**
   * @brief set the valid time of the key
   * @param time: the valid time
   * @return void
   */
  void SetKeyValidTime(std::string time) {
    key_valid_time_ = time;
  }

  /**
   * @brief set the valid time of the token
   * @param time: the valid time
   * @return void
   */
  void SetTokenValidTime(std::string time) {
    token_valid_time_ = time;
  }

 private:
  /**
   * @brief constructor
   */
  AuthPara(const AuthPara &auth_para);

  /**
   * @brief operator=
   */
  void operator=(const AuthPara &auth_para);

  const std::string auth_url_;
  std::string key_;
  std::string token_;
  std::string key_valid_time_;
  std::string token_valid_time_;
};
}  // namespace v2x
#endif /* AUTH_PARA_H_ */
