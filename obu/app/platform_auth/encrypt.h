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

#ifndef PLATFORM_AUTH_ENCRYPT_H
#define PLATFORM_AUTH_ENCRYPT_H

#include "openssl/aes.h"
#include "openssl/bio.h"
#include "openssl/buffer.h"
#include "openssl/evp.h"
#include "openssl/md5.h"

/**
 * @brief encrypt
 * @param in: plain text input
 *        len: length of in
 *        key: the key
 *        out: cipher text
 * @return if encryption successes, 1 returned, if not, 0 returned
 */
int AesEncrypt(unsigned char *in, int len, unsigned char *key,
               unsigned char *out) {
  if (!in || !key || !out) return 0;
  AES_KEY aes;
  if (AES_set_encrypt_key(key, 128, &aes) < 0) {
    return 0;
  }

  int en_len = 0;
  while (en_len < len) {
    AES_encrypt(in, out, &aes);
    in += AES_BLOCK_SIZE;
    out += AES_BLOCK_SIZE;
    en_len += AES_BLOCK_SIZE;
  }
  return 1;
}

/**
 * @brief decrypt
 * @param in: cipher text input
 *        len: length of in
 *        key: the key
 *        out: plain text
 * @return if decryption successes, 1 returned, if not, 0 returned
 */
int AesDecrypt(unsigned char *in, int len, unsigned char *key,
               unsigned char *out) {
  if (!in || !key || !out) return 0;
  AES_KEY aes;
  if (AES_set_decrypt_key(key, 128, &aes) < 0) {
    return 0;
  }

  int en_len = 0;
  while (en_len < len) {
    AES_decrypt(in, out, &aes);
    if ((en_len+AES_BLOCK_SIZE) <= len) {
    in += AES_BLOCK_SIZE;
    out += AES_BLOCK_SIZE;
    en_len += AES_BLOCK_SIZE;
    } else {
      in += (len % AES_BLOCK_SIZE);
      out+= (len % AES_BLOCK_SIZE);
      en_len += (len % AES_BLOCK_SIZE);
    }
  }
  return 1;
}

/**
 * @brief get the MD5 value with 32 bytes
 * @param input: plain text input
 *        buf: MD5 value buffer
 * @return if encryption successes, 1 returned, if not, 0 returned
 */
int MD5Encode(const char *input, char *buf) {
  unsigned char md[16];
  char tmp[3] = {'\0'};
  int i = 0;

  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, input, strlen(input));
  MD5_Final(md, &ctx);

  for (i = 0; i < 16; i++) {
    sprintf(tmp, "%02X", md[i]);
    strcat(buf, tmp);
  }

  return 1;
}

/**
 * @brief encode to base64 value
 * @param buf: plain text buffer
 *        length: the length of buffer
 *        output: the buffer for encoding
 * @return the length of the output buffer
 */
size_t Base64Encode(const char *buffer, int length, char* output) {
  BIO *bmem = NULL;
  BIO *b64 = NULL;
  BUF_MEM *bptr;
  
  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_write(b64, buffer, length);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bptr);
  BIO_set_close(b64, BIO_NOCLOSE);

  memcpy(output, bptr->data, bptr->length);
  output[bptr->length] = '\0';
  
  BIO_free_all(b64);

  return bptr->length;;
}

/**
 * @brief decode from base64 value
 * @param input: base64 buffer
 *        length: the length of buffer
 *        output: output buffer
 * @return the length of the output buffer
 */
int Base64Decode(const char *input, int length, char* output) {
  BIO *b64 = NULL;
  BIO *bmem = NULL;
  int size  = 0;
  //char *buffer = (char *)malloc(length);
  //memset(buffer, 0, length);
  b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  bmem = BIO_new_mem_buf((void *)input, length);
  bmem = BIO_push(b64, bmem);
  size = BIO_read(bmem, output, length);
  output[size] = '\0';

  BIO_free_all(bmem);

  return size;
}

#endif /* PLATFORM_AUTH_ENCRYP_ENCRYPT_H */
