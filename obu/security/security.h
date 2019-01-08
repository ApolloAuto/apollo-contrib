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
 * @file security.h
 * @brief This is for data security. It will add a header and a tail to input
 * data. The header includes CA and the tail includes signature.
 * The functions:
 * Secured: add signature.
 * VerifyAndStrip: verify signature then strip the header and tail.
 *
 * The public key cryptographic algorithm we use is SM2.
 * The description is at
 * http://www.sca.gov.cn/sca/xwdt/2010-12/17/1002386/files/b791a9f908bb4803875ab6aeeb7b4e03.pdf.
 * The cryptographic hash algorithm is SM3 and the description is at
 * http://www.sca.gov.cn/sca/xwdt/2010-12/17/1002389/files/302a3ada057c4a73830536d03e683110.pdf.
 * The certification format is followed the standard
 * "Intelligent transport - Certificate application interface". The document is at http://www.mot.gov.cn/yijianzhengji/lishizhengji/201711/P020171101362371804539.pdf.
 *
 * Note: you must call Init first.
 */

#ifndef APOLLO_V2X_SECURITY_SECURITY_H
#define APOLLO_V2X_SECURITY_SECURITY_H

#include <openssl/ec.h>
#include <string>
#include "SecuredMessage.h"

namespace v2x {

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  TypeName& operator=(const TypeName&)

class Security {
 public:
  explicit Security(std::string root_ca_file, std::string self_ca_file);
  ~Security();

  /**
   * @brief Initialize.
   * @return return 0 successed,
   *                -1 failed
   */
  int Init();

  /**
   * @brief Sign the data.
   * @param data (input)
   * @param data_len (input)
   * @param sign_data (output)
   * @param sign_len (output)
   */
  virtual void Sign(uint8_t* data, size_t data_len, uint8_t* sign_data,
                    size_t* sign_len);

  /**
   * @brief Sign and add a header and tail(includes signature) the data.
   * @param data (input)
   * @param data_len (input)
   * @param secured_data (output)
   * @param secured_len (output)
   */
  virtual void Secured(uint8_t* data, size_t data_len, uint8_t* secured_data,
                       size_t secured_buf_size, size_t* secured_len);

  /**
   * @brief Verify the signature, strip the header and tail.
   * @param secured_data (input)
   * @param secured_len (input)
   * @param data (output)
   * @param data_len (output)
   * @return 0  not secured or verified successfully
   *         -1 parameters error
   *         -2 decode failed
   *         -3 get public key in the data failed
   *         -4 verify failed
   */
  virtual int VerifyAndStrip(uint8_t* secured_data, size_t secured_len,
                             uint8_t* data, size_t* data_len);

 private:
  DISALLOW_COPY_AND_ASSIGN(Security);

  std::string root_ca_file_;
  std::string self_ca_file_;

  uint8_t self_ca_buf_[1024] = {0};
  size_t self_ca_len_;
  EC_KEY* root_key_;
  EC_KEY* self_key_;
};

}  // namespace v2x

#endif  // APOLLO_V2X_SECURITY_SECURITY_H
