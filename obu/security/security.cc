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
 * @file Security.cc
 * @brief This is for data security. It will add a header and a tail to input
 * data. The header includes CA and the tail includes signature.
 * The functions:
 * Secured: add header and tail.
 * VerifyAndStrip: verify signature then strip the header and tail.
 *
 * Note: you must call Init first.
 */

#include "security.h"
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "common.h"
#include "glog/logging.h"
#include "sm2.h"
#include "sm3.h"

namespace v2x {

Security::Security(std::string root_ca_file, std::string self_ca_file)
    : root_ca_file_(root_ca_file),
      self_ca_file_(self_ca_file),
      root_key_(nullptr),
      self_key_(nullptr) {}

Security::~Security() {
  if (root_key_ != nullptr) {
    EC_KEY_free(root_key_);
  }
  if (self_key_ != nullptr) {
    EC_KEY_free(self_key_);
  }
}

int Security::Init() {
  if(!root_ca_file_.empty()) {
    root_key_ = GetKey(root_ca_file_.c_str());
    if (root_key_ == nullptr) {
      LOG(ERROR) << "Get root key failed";
      return -1;
    }
  }

  if(!self_ca_file_.empty()) {
    int ret = 0;

    self_key_ = GetKey(self_ca_file_.c_str());
    if (self_key_ == nullptr) {
      LOG(ERROR) << "Get self key failed";
      return -1;
    }
    ret = GetCert(self_ca_file_.c_str(), self_ca_buf_, &self_ca_len_);
    if (ret != 0) {
      LOG(ERROR) << "Get self certification failed";
      return -1;
    }
  }

  return 0;
}

void Security::Sign(uint8_t *data, size_t data_len, uint8_t *sign_data,
                    size_t *sign_len) {
  uint8_t digest[64] = {0};

  SM3(data, data_len, digest);
  SM2Sign(digest, 32, sign_data, sign_len, self_key_);
}

void Security::Secured(uint8_t *data, size_t data_len, uint8_t *secured_data,
                       size_t secured_buf_size, size_t *secured_len) {
  SecuredMessage_t secured_msg;
  uint8_t signature[128] = {0};
  size_t sig_len = 0;

  memset(&secured_msg, 0, sizeof(secured_msg));
  secured_msg.protocolVersion = 2;

  secured_msg.headerFields.signerinfo =
      (SignerInfo_t *)malloc(sizeof(SignerInfo_t));
  memset(secured_msg.headerFields.signerinfo, 0, sizeof(SignerInfo_t));
  secured_msg.headerFields.signerinfo->present = SignerInfo_PR_certificateStr;
  secured_msg.headerFields.signerinfo->choice.certificateStr.size =
      self_ca_len_;
  secured_msg.headerFields.signerinfo->choice.certificateStr.buf =
      (uint8_t *)malloc(self_ca_len_);
  memcpy(secured_msg.headerFields.signerinfo->choice.certificateStr.buf,
         self_ca_buf_, self_ca_len_);

  secured_msg.payloadField.present = Payload_PR_signed;
  secured_msg.payloadField.choice.Signed.size = data_len;
  secured_msg.payloadField.choice.Signed.buf = (uint8_t *)malloc(data_len);
  memcpy(secured_msg.payloadField.choice.Signed.buf, data, data_len);

  Sign(data, data_len, signature, &sig_len);
  secured_msg.trailerFields.present = TrailerField_PR_signature;
  secured_msg.trailerFields.choice.signature.present = Signature_PR_signature;
  secured_msg.trailerFields.choice.signature.choice.signature.size = sig_len;
  secured_msg.trailerFields.choice.signature.choice.signature.buf =
      (uint8_t *)malloc(sig_len);
  memcpy(secured_msg.trailerFields.choice.signature.choice.signature.buf,
         signature, sig_len);

  asn_enc_rval_t ret;
  ret = der_encode_to_buffer(&asn_DEF_SecuredMessage, &secured_msg,
                             secured_data, secured_buf_size);
  *secured_len = ret.encoded;
}

int Security::VerifyAndStrip(uint8_t *secured_data, size_t secured_len,
                             uint8_t *data, size_t *data_len) {
  SecuredMessage_t *secured_msg = nullptr;
  EC_KEY *pub_key = NULL;
  uint8_t digest[64] = {0};

  if (secured_data == nullptr || data == nullptr || data_len == nullptr) {
    LOG(ERROR) << "Parameters error";
    return -1;
  }

  ber_decode(NULL, &asn_DEF_SecuredMessage, (void **)&secured_msg, secured_data,
             secured_len);
  if (secured_msg == nullptr) {
    return -2;
  }
  pub_key =
      GetPubKey(secured_msg->headerFields.signerinfo->choice.certificateStr.buf,
                secured_msg->headerFields.signerinfo->choice.certificateStr.size,
                root_key_);
  if (pub_key == NULL) {
    return -3;
  }

  SM3(secured_msg->payloadField.choice.Signed.buf,
      secured_msg->payloadField.choice.Signed.size, digest);
  if (!SM2Verify(
          digest, 32,
          secured_msg->trailerFields.choice.signature.choice.signature.buf,
          secured_msg->trailerFields.choice.signature.choice.signature.size,
          pub_key)) {
    EC_KEY_free(pub_key);
    return -4;
  }

  memcpy(data, secured_msg->payloadField.choice.Signed.buf,
         secured_msg->payloadField.choice.Signed.size);
  *data_len = secured_msg->payloadField.choice.Signed.size;

  EC_KEY_free(pub_key);
  return 0;
}

}  // End of namespace v2x
