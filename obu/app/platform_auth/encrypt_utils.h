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
 * @brief This library provides the basic encryption and decryption utils,
 * including aes, md5 and base64.
 */

#ifndef PLATFORM_AUTH_ENCRYPT_UTILS_H
#define PLATFORM_AUTH_ENCRYPT_UTILS_H

#include <string>

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {

/**
 * @class AESECButil
 *
 * @brief aes ecb encryption utils
 */
class AESECButil {
 public:
  /**
   * @brief constructor
   */
  AESECButil() {}

  /**
   * @brief destructor
   */
  ~AESECButil() {}

  /**
   * @brief encrypt with PKCS5Padding
   * @param content: plain text
   *        key: the key
   * @return if encryption successes, cipher text returned, if not, the plain
   * text returned
   */
  static std::string EncryptPKCS5(std::string content, std::string key);

  /**
   * @brief encrypt with PKCS5Padding
   * @param content: pointer to plain text
   *        length: length of the content
   *        out: pointer to the cipher
   *        key: the key
   * @return if encryption successes, the length of cipher returned , if not, 0
   * returned
   */
  static int EncryptPKCS5(unsigned char *content, int length,
                          unsigned char *out, std::string key);

  /**
   * @brief encrypt with PKCS5Padding
   * @param content: cipher text
   *        key: the key
   * @return if decryption successes, plain text returned, if not, the cipher
   * text returned
   */
  static std::string DecryptPKCS5(std::string content, std::string key);

  /**
   * @brief encrypt with PKCS5Padding
   * @param content: pointer to cipher text
   *        length: length of the content
   *        out: pointer to the plain text
   *        key: the key
   * @return if decryption successes, the length of plain text returned , if
   * not, 0 returned
   */
  static int DecryptPKCS5(unsigned char *content, int length,
                          unsigned char *out, std::string key);

 private:
  /**
   * @brief constructor
   */
  AESECButil(const AESECButil &aes);

  /**
   * @brief operator=
   */
  void operator=(const AESECButil &aes);
};

/**
 * @class MD5util
 *
 * @brief MD5 utils
 */
class MD5util {
 public:
  /**
   * @brief constructor
   */
  MD5util() {}

  /**
   * @brief constructor
   */
  ~MD5util() {}

  /**
   * @brief get the MD5 value with 32 bytes
   * @param input: plain text
   * @return the MD5 value
   */
  static std::string GetMD532(std::string input);

  /**
   * @brief get the MD5 value with 16 bytes
   * @param input: plain text
   * @return the MD5 value
   */
  static std::string GetMD516(std::string input);

 private:
  /**
   * @brief constructor
   */
  MD5util(const MD5util &md5);

  /**
   * @brief operator=
   */
  void operator=(const MD5util &md5);
};

/**
 * @class Base64util
 *
 * @brief base64 utils
 */
class Base64util {
 public:
  /**
   * @brief constructor
   */
  Base64util() {}

  /**
   * @brief denstructor
   */
  ~Base64util() {}

  /**
   * @brief get the base64 value
   * @param input: plain text
   * @return the base64 value
   */
  static std::string GetBase64(std::string input);

  /**
   * @brief parse the base64 value
   * @param input: base64 value
   * @return plain text
   */
  static std::string ParseBase64(std::string input);

 private:
  /**
   * @brief constructor
   */
  Base64util(const Base64util &base64);

  /**
   * @brief operator=
   */
  void operator=(const Base64util &base64);
};
}  // namespace v2x
#endif /* PLATFORM_AUTH_ENCRYPT_UTILS_H */

