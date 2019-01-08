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

#include "encrypt_utils.h"
#include <stdlib.h>
#include <string.h>
#include <memory>
#include "encrypt.h"
#include <iostream>
#include "glog/logging.h"
namespace v2x {

/**
 * @class AESECButil
 *
 * @brief aes ecb encryption utils
 */

std::string AESECButil::EncryptPKCS5(std::string content, std::string key) {
  // PKCS5Padding
  int n = 16 - (content.length() % 16);
  std::string padding;
  int i = 0;
  char tmp = n;
  for (i = 0; i < n; i++) {
    padding.push_back(tmp);
  }

  std::string input = content + padding;
  std::string out;

  unsigned char *uc_input = new unsigned char[input.length() + 1];
  unsigned char *uc_key = new unsigned char[key.length() + 1];
  unsigned char *uc_out = new unsigned char[input.length() + 1];

  memset(uc_input, 0x00, input.length() + 1);
  memset(uc_key, 0x00, key.length() + 1);
  memset(uc_out, 0x00, content.length());

  memcpy(uc_input, input.c_str(), input.length());
  memcpy(uc_key, key.c_str(), key.length());

  if (AesEncrypt(uc_input, input.length(), uc_key, uc_out)) {
    unsigned int i = 0;
    for (i = 0; i < input.length(); i++) {
      out.push_back(uc_out[i]);
    }
    delete[] uc_input;
    delete[] uc_key;
    delete[] uc_out;
    return out;
  } else {  // if encryption failed, return the content
    delete[] uc_input;
    delete[] uc_key;
    delete[] uc_out;
    return content;
  }
}

int AESECButil::EncryptPKCS5(unsigned char *content, int length,
                             unsigned char *out, std::string key) {
  // PKCS5Padding
  int n = 16 - (length % 16);
  int i = 0;
  unsigned char tmp = n;
  for (i = 0; i < n; i++) {
    memcpy(content + length + i, &tmp, 1);
  }

  std::shared_ptr<void> uc_key(malloc(key.length()), free);

  memset(uc_key.get(), 0x00, key.length());
  memcpy(uc_key.get(), key.c_str(), key.length());

  if (AesEncrypt(content, length + n, reinterpret_cast<unsigned char*>(uc_key.get()), out)) {
    return length + n;
  } else {  // if encryption failed, return 0
    return 0;
  }
}

std::string AESECButil::DecryptPKCS5(std::string content, std::string key) {
  char *uc_out = new char[content.length()];

  memset(uc_out, 0x00, content.length());

  if (AesDecrypt(( unsigned char *)(content.c_str()), content.length(), (unsigned char *)(key.c_str()), (unsigned char*)uc_out) == 1) {
    std::string out(uc_out, content.length() - uc_out[content.length() - 1]);
    delete[] uc_out;
    return out;
  } else {
    delete[] uc_out;
    return content;
  }
}

int AESECButil::DecryptPKCS5(unsigned char *content, int length,
                             unsigned char *out, std::string key) {
  if (AesDecrypt(content, length, (unsigned char *)key.c_str(), out)) {
    char padding = *(out + length - 1);
    return length - padding;
  } else {
    return 0;
  }
}

/**
 * @class MD5util
 *
 * @brief MD5 utils
 */

std::string MD5util::GetMD532(std::string input) {
  char buf[33] = {'\0'};
  MD5Encode(input.c_str(), buf);

  return buf;
}

std::string MD5util::GetMD516(std::string input) {
  return GetMD532(input).substr(8, 16);
}

/**
 * @class Base64util
 *
 * @brief base64 utils
 */

std::string Base64util::GetBase64(std::string input) {
  uint32_t byte_num = (input.length()*8 + 24 - input.length()*8 % 24)/6;
  char *buf = reinterpret_cast<char*>(malloc(byte_num));
  if (buf == nullptr) {
    LOG(ERROR) << "Alloc the memory failed";
     return input;
  }
  size_t len = Base64Encode(input.c_str(), input.length(), buf);
  std::string out(buf, len);
  free(buf);

  return out;
}

std::string Base64util::ParseBase64(std::string input) {
  int output_size = 0;
  char* buf = (char*)malloc(input.length());
  if (buf == nullptr) {
     LOG(ERROR) << "Alloc the memory failed";
     return input;
  }
  output_size = Base64Decode(input.c_str(), input.length(), buf);
  std::string out(buf, output_size);
  free(buf);

  return out;
}
}  // namespace v2x

