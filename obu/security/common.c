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
 * @file common.c
 * @brief This is a common file for v2x security.
 */

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/buffer.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "Certificate.h"
#include "ToBeSigned.h"
#include "common.h"
#include "sm2.h"
#include "sm3.h"

const char *key_title = "-----BEGIN PRIVATE KEY-----\n";
const char *key_end = "-----END PRIVATE KEY-----\n";
const char *cert_title = "-----BEGIN CERTIFICATE-----\n";
const char *cert_end = "-----END CERTIFICATE-----\n";

time_t GetTimeSec(int year, int month, int day, int hour, int min, int sec) {
  struct tm tmval;
  time_t tmsec;

  memset(&tmval, 0, sizeof(tmval));

  tmval.tm_year = year;
  tmval.tm_mon = month, tmval.tm_mday = day;
  tmval.tm_hour = hour;
  tmval.tm_min = min;
  tmval.tm_sec = sec;

  tmsec = mktime(&tmval);
  return tmsec;
}

void printChar(uint8_t *buffer, int len) {
  int i = 0;

  if (buffer == NULL) {
    return;
  }

  printf("len %d\n", len);
  for (i = 0; i < len; i++) {
    printf("%02x ", buffer[i]);
  }
  printf("\n");
}

void Base64Encode(const uint8_t *input, int length, uint8_t *output,
                  int *out_len) {
  EVP_ENCODE_CTX ectx;
  int total_len = 0;
  int tmp_len = 0;
  int size = length * 2;

  if (input == NULL || output == NULL || out_len == NULL) {
    printf("Parameters error\n");
    return;
  }
  size = size > 64 ? size : 64;
  EVP_EncodeInit(&ectx);
  EVP_EncodeUpdate(&ectx, output, &tmp_len, input, length);
  total_len += tmp_len;
  EVP_EncodeFinal(&ectx, output + total_len, &tmp_len);
  total_len += tmp_len;
  *out_len = total_len;
}

void Base64Decode(const uint8_t *input, int length, uint8_t *output,
                  int *out_len) {
  EVP_ENCODE_CTX ectx;
  int total_len = 0;
  int tmp_len = 0;

  if (input == NULL || output == NULL || out_len == NULL) {
    printf("Parameters error\n");
    return;
  }

  EVP_DecodeInit(&ectx);
  EVP_DecodeUpdate(&ectx, output, &tmp_len, input, length);
  total_len += tmp_len;
  EVP_DecodeFinal(&ectx, output + total_len, &tmp_len);
  total_len += tmp_len;
  *out_len = total_len;
}

static EC_KEY *SetupKey(char *pri_key, char *pub_key_x, char *pub_key_y)
{
  EC_KEY *eckey = NULL;
  BIGNUM *x = NULL;
  BIGNUM *y = NULL;
  BIGNUM *z = NULL;

  y = BN_new();
  z = BN_new();

  if (!y || !z) {
    printf("New bn failed\n");
    goto end;
  }
  if (pri_key != NULL) {
    x = BN_new();
    if (!x) {
      printf("New bn failed\n");
      goto end;
    }
  }
  BN_bin2bn(pri_key, 32, x);
  BN_bin2bn(pub_key_x, 32, y);
  BN_bin2bn(pub_key_y, 32, z);
  eckey = SM2SetupKey(x, y, z);
  if (eckey == NULL) {
    printf("Setup key fail\n");
  }
end:
  if (x != NULL) {
    BN_free(x);
  }
  if (y != NULL) {
    BN_free(y);
  }
  if (z != NULL) {
    BN_free(z);
  }

  return eckey;
}

static EC_KEY *SetupKeyFromCA(char * ca, int ca_len, char * pri_key) {
  Certificate_t *cert = NULL;
  EC_KEY *eckey = NULL;
  ber_decode(NULL, &asn_DEF_Certificate, (void **)&cert, ca, ca_len);
  if (cert == NULL) {
    printf("Get Public Key failed\n");
    return NULL;
  }

  eckey = SetupKey(pri_key, cert->subjectAttributes.verificationKey->choice.signKey.buf,
                   cert->subjectAttributes.verificationKey->choice.signKey.buf + 32);
  SEQUENCE_free(&asn_DEF_Certificate, cert, 0);

  return eckey;
}

EC_KEY *GetKey(const char *ca_file) {
  char buf[1024] = {0};
  char b64[512] = {0};
  char ca[512] = {0};
  FILE *f = NULL;
  Certificate_t *cert = NULL;
  EC_KEY *eckey = NULL;

  if (ca_file == NULL) {
    printf("Parameters error\n");
    return NULL;
  }

  f = fopen(ca_file, "r");
  if (f == NULL) {
    printf("Open file %s failed\n", ca_file);
    return NULL;
  }
  fread(buf, sizeof(buf) - 1, 1, f);

  // Get Private Key
  char *r = strstr(buf, key_title);
  char *s = strstr(buf, key_end);
  char ckey[128] = {0};
  char key[128] = {0};
  int key_len = 0;
  if (r != NULL && s != NULL) {
    int ckey_len = s - r - strlen(key_title);
    strncpy(ckey, r + strlen(key_title), ckey_len);
    Base64Decode((uint8_t *)ckey, ckey_len, (uint8_t *)key, &key_len);
  }

  // Get Public Key
  char *p = strstr(buf, cert_title);
  char *q = strstr(buf, cert_end);
  int b64_len = q - p - strlen(cert_title);
  int ca_len = 0;
  if (p == NULL || q == NULL) {
    printf("Fail read public key\n");
    fclose(f);
    return NULL;
  }
  strncpy(b64, p + strlen(cert_title), b64_len);
  Base64Decode((uint8_t *)b64, b64_len, (uint8_t *)ca, &ca_len);
  fclose(f);

  return SetupKeyFromCA(ca, ca_len, key);
}

int GetCert(const char *ca_file, uint8_t *cert, size_t *cert_len) {
  char buf[1024] = {0};
  char b64[512] = {0};
  FILE *f = NULL;

  if (ca_file == NULL || cert == NULL || cert_len == NULL) {
    printf("Parameters error\n");
    return -1;
  }

  f = fopen(ca_file, "r");
  if (f == NULL) {
    printf("Open file %s failed\n", ca_file);
    return -1;
  }

  fread(buf, sizeof(buf) - 1, 1, f);

  char *p = strstr(buf, cert_title);
  char *q = strstr(buf, cert_end);
  if (p == NULL || q == NULL) {
    printf("Could not find certification\n");
    fclose(f);
    return -1;
  }
  int b64_len = q - p - strlen(cert_title);
  strncpy(b64, p + strlen(cert_title), b64_len);
  Base64Decode((uint8_t *)b64, b64_len, cert, cert_len);

  fclose(f);
  return 0;
}

EC_KEY *GetPubKey(uint8_t *ca_buf, int ca_len, EC_KEY *root_key) {
  char buf[1024] = {0};
  char tbs_data[512] = {0};
  char signature[128] = {0};
  char digest[32] = {0};
  int buf_len = 0;
  int sig_len = 0;
  int rc = 0;
  Certificate_t *cert = NULL;
  ToBeSigned_t *tbs = NULL;
  EC_KEY *self_key = NULL;
  asn_enc_rval_t ret;
  BIGNUM *y = NULL;
  BIGNUM *z = NULL;

  if (ca_buf == NULL || root_key == NULL) {
    printf("Parameters error\n");
    goto end;
  }

  ber_decode(NULL, &asn_DEF_Certificate, (void **)&cert, ca_buf, ca_len);
  if (cert == NULL) {
    printf("Decode CA failed\n");
    goto end;
  }

  ber_decode(NULL, &asn_DEF_ToBeSigned, (void **)&tbs, ca_buf, ca_len);
  if (tbs == NULL) {
    printf("Decode CA failed\n");
    goto end;
  }

  ret = der_encode_to_buffer(&asn_DEF_ToBeSigned, tbs, tbs_data,
                             sizeof(tbs_data));
  if (ret.encoded == 0) {
    printf("Encode failed\n");
    goto end;
  }
  SM3(tbs_data, ret.encoded, digest);
  rc = SM2Verify(digest, 32, cert->signature.choice.signature.buf,
                 cert->signature.choice.signature.size, root_key);
  if (rc != 1) {
    printf("Verify failed. Ret %d\n", rc);
    goto end;
  }

  // Setup eckey
  y = BN_new();
  z = BN_new();

  if (!y || !z) {
    printf("New bn failed\n");
    goto end;
  }
  BN_bin2bn(cert->subjectAttributes.verificationKey->choice.signKey.buf, 32, y);
  BN_bin2bn(cert->subjectAttributes.verificationKey->choice.signKey.buf + 32,
            32, z);
  self_key = SM2SetupKey(NULL, y, z);
  if (self_key == NULL) {
    printf("Setup key fail\n");
    goto end;
  }

end:
  if (cert != NULL) {
    SEQUENCE_free(&asn_DEF_Certificate, cert, 0);
  }
  if (tbs != NULL) {
    SEQUENCE_free(&asn_DEF_ToBeSigned, tbs, 0);
  }
  if (y != NULL) {
    BN_free(y);
  }
  if (z != NULL) {
    BN_free(z);
  }

  return self_key;
}

int WriteToFile(char *ca_file, Certificate_t *cert, const char *pri_key,
                 const char *signer_info_digest) {
  const char *sign_digest_title = "-----BEGIN SIGNATURE DIGEST-----\n";
  const char *sign_digest_end = "-----END SIGNATURE DIGEST-----\n";
  FILE *f;
  uint8_t ca[256] = {0};
  uint8_t b64[512] = {0};
  uint8_t tmp[512] = {0};
  int b64_len;
  int out_len;
  asn_enc_rval_t ret;

  if (ca_file == NULL || cert == NULL || pri_key == NULL) {
    printf("Parameters error\n");
    return -1;
  }
  f = fopen(ca_file, "w");
  if (f == NULL) {
    printf("Open file %s failed\n", ca_file);
    return -1;
  }

  ret = der_encode_to_buffer(&asn_DEF_Certificate, cert, ca, sizeof(ca));
  if (ret.encoded == 0) {
    fclose(f);
    printf("Encode certification failed\n");
    return -1;
  }

  Base64Encode(ca, ret.encoded, b64, &b64_len);
  Base64Encode(pri_key, strlen(pri_key), tmp, &out_len);
  fwrite(key_title, strlen(key_title), 1, f);
  fwrite(tmp, out_len, 1, f);
  fwrite(key_end, strlen(key_end), 1, f);
  fwrite(cert_title, strlen(cert_title), 1, f);
  fwrite(b64, b64_len, 1, f);
  fwrite(cert_end, strlen(cert_end), 1, f);
  if (signer_info_digest != NULL) {
    fwrite(sign_digest_title, strlen(sign_digest_title), 1, f);
    fwrite(signer_info_digest, strlen(signer_info_digest), 1, f);
    fwrite(sign_digest_end, strlen(sign_digest_end), 1, f);
  }
  fclose(f);

  return 0;
}
