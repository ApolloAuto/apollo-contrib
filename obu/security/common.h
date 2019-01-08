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
 * @file common.h
 * @brief This is a common file for v2x security.
 */

#ifndef APOLLO_V2X_SECURITY_COMMON_H
#define APOLLO_V2X_SECURITY_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the second by time. It's the second after 1970, Jan 1st, 00:00:00.
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param min
 * @param sec
 * @return second count
 */
time_t GetTimeSec(int year, int month, int day, int hour, int min, int sec);

/**
 * @brief Print string with %02x format.
 * @param buffer
 * @param len
 */
void printChar(uint8_t *buffer, int len);

/**
 * @brief Encode data with base64 format.
 * @param input
 * @param inlen
 * @param output
 * @param outlen
 */
void Base64Encode(const uint8_t *input, int inlen, uint8_t *output,
                  int *outlen);

/**
 * @brief Decode base64 format data.
 * @param input
 * @param inlen
 * @param output
 * @param outlen
 */
void Base64Decode(const uint8_t *input, int inlen, uint8_t *output,
                  int *outlen);

/**
 * @brief Get ECKEY from CA file. It must has public key, but private key is optional.
 * @param ca_file CA file name
 * @return EC_KEY eckey or NULL
 */
EC_KEY *GetKey(const char *ca_file);

/**
 * @brief Get content of the CA file.
 * @param ca_file CA file name
 * @param cert certification content
 * @param cert_len
 * @return 0 for success -1 for fail
 */
int GetCert(const char *ca_file, uint8_t *cert, size_t *cert_len);

/**
 * @brief Get public key from a ca. The public key would be verified by root key
 * @param ca_buf CA buffer
 * @param ca_len
 * @param root_key root CA key
 * @return eckey or NULL
 */
EC_KEY *GetPubKey(uint8_t *ca_buf, int ca_len, EC_KEY *root_key);

/**
 * @brief Write certification to CA file.
 * @param ca_file CA file name
 * @param cert certification structure
 * @param pri_key private key
 * @param signer_info_digest optional signer certification digest(8 Bytes)
 * @return 0 for succes -1 for fail
 */
int WriteToFile(char *ca_file, Certificate_t *cert, const char *pri_key,
                const char *signer_info_digest);

#ifdef __cplusplus
}
#endif

#endif  // APOLLO_V2X_SECURITY_COMMON_H
