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

#include "sys/time.h"
#include "time.h"

#include "auth_manager.h"

#include "cJSON.h"
#include "curl.h"
#include "encrypt_utils.h"
#include "glog/logging.h"
namespace v2x {

/**
 * @brief format an integer to decmical string
 * @param input: the integer to be formatted
 *        size: number of the integer
 * @return interger formatted
 */
std::string ToDecString(int input, int size) {
  if (size > 6) {
    size = 6;
  }

  char format[5];
  sprintf(format, "%c", '%');
  sprintf(format + 1, "%d", 0);
  sprintf(format + 2, "%d", size);
  sprintf(format + 3, "%c", 'd');
  format[4] = '\0';

  char tmp[6];
  sprintf(tmp, format, input);
  std::string out(tmp, size);
#if 0
  int i = 0;
  for (i = 0; i < size; i++) {
    out.push_back(tmp[i]);
  }
#endif
  return out;
}

/**
 * @brief write a buffer to string
 * @param buffer: pointer to buffer to be written to string
 * @return interger formatted
 */
static size_t WriteToString(void *buffer, size_t size, size_t nmemb,
                            void *userp) {
  *((std::string *)userp) += (char *)buffer;
  return size * nmemb;
}

/**
 * @class BaseAuthManager
 *
 * @brief base class of authentication manager
 */
BaseAuthManager *BaseAuthManager::Create(int vendor_code, AuthPara &auth_para,
                                         DeviceInfo &device_info) {
  switch (vendor_code) {
    case (int)(PLATFORM_VENDOR::CMCC_V2X):
      return new AuthManager(auth_para, device_info);
    default:
      break;
  }

  return nullptr;
}

/**
 * @class AuthManager
 *
 * @brief auth manager for platform
 */
AuthManager::AuthManager(AuthPara &auth_para, DeviceInfo &device_info)
    : auth_para_(auth_para), device_info_(device_info), auth_status_(NONE) {}

bool AuthManager::DeviceRegister() {
  // register off line
  // reserved

  return true;
}

bool AuthManager::DeviceAuth() {
  std::string request_body = this->RequestBody();
  // Http
  CURL *curl = curl_easy_init();
  std::string token;

  if (curl) {
    std::string url = auth_para_.GetAuthUrl();
    struct curl_slist *header = nullptr;

    header = curl_slist_append(header,
                               "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    std::string str_data;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&str_data);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    // curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
    if (res == CURLcode::CURLE_OK) {
      // std::cout << "str_data= " << str_data << std::endl;
      LOG(INFO) << "str_data= " << str_data;
    } else {
      LOG(ERROR) << "curl POST failed!";
      return false;
    }

    size_t pos = str_data.find("{");
    if (ParseResponseBody(str_data.substr(pos, str_data.size() - pos))) {
      return true;
    } else {
      LOG(ERROR) << "parse the http response failed!";
      return false;
    }
  } else {
    LOG(ERROR) << "curl is null";
    return false;
  }
}

void AuthManager::UpdateAuthStatus() {
  // check token valid time
  if (CheckTokenValid() == 0) {  // invalid token
    SetAuthStatus(/*AUTH_STATUS::*/ INVALID_TOKEN);
    if (CheckKeyValid() == 0) {
      SetAuthStatus(AUTH_STATUS::INVALID_ALL);
    }
    DeviceAuth();
  } else if (CheckKeyValid() == 0) {
    SetAuthStatus(AUTH_STATUS::INVALID_KEY);
    DeviceAuth();
  }
}

AUTH_STATUS AuthManager::GetAuthStatus() {
  return auth_status_;
}

void AuthManager::SetAuthStatus(AUTH_STATUS status) {
  auth_status_ = status;
}

std::string AuthManager::CurrentTime() {
  struct timeval cmtm;
  gettimeofday(&cmtm, NULL);
  struct tm *ctm = localtime(&(cmtm.tv_sec));

  return ToDecString(ctm->tm_year + 1900, 4) + ToDecString(ctm->tm_mon + 1, 2) +
         ToDecString(ctm->tm_mday, 2) + ToDecString(ctm->tm_hour, 2) +
         ToDecString(ctm->tm_min, 2) + ToDecString(ctm->tm_sec, 2) +
         ToDecString((int)(cmtm.tv_usec / 1000), 3);
}

struct timeval AuthManager::ParseStringTime(std::string time) {
  struct tm the_tm = {0};
  int the_mili_sec = 0;
  sscanf(time.c_str(), "%4d%2d%2d%2d%2d%2d%3d", &(the_tm.tm_year),
         &(the_tm.tm_mon), &(the_tm.tm_mday), &(the_tm.tm_hour),
         &(the_tm.tm_min), &(the_tm.tm_sec), &(the_mili_sec));

  the_tm.tm_year -= 1900;  // years from 1900
  the_tm.tm_mon--;         // 0 ~ 11

  timeval time_val;
  time_val.tv_sec = mktime(&the_tm);
  time_val.tv_usec = the_mili_sec * 1000;

  return time_val;
}

time_t AuthManager::SecFromValidTime(std::string valid_time) {
  struct timeval valid_micro_time = ParseStringTime(valid_time);

  struct timeval current_micro_tm;
  gettimeofday(&current_micro_tm, NULL);

  return valid_micro_time.tv_sec - current_micro_tm.tv_sec + 1;
}

bool AuthManager::CheckValidTime(std::string valid_time) {
  struct timeval valid_micro_time = ParseStringTime(valid_time);

  struct timeval current_micro_tm;
  gettimeofday(&current_micro_tm, NULL);

  if (valid_micro_time.tv_sec - current_micro_tm.tv_sec < 0) {
    return false;
  } else if (valid_micro_time.tv_sec - current_micro_tm.tv_sec == 0) {
    if (valid_micro_time.tv_usec - current_micro_tm.tv_usec <= 0) {
      return false;
    } else {
      return true;
    }
  } else {
    return true;
  }
}

bool AuthManager::CheckKeyValid() {
  return CheckValidTime(auth_para_.GetKeyValidTime());
}

bool AuthManager::CheckTokenValid() {
  return CheckValidTime(auth_para_.GetTokenValidTime());
}

std::string AuthManager::RequestBody() {
  std::string id = device_info_.GetDeviceId();
  std::string mac = device_info_.GetMac();
  int req_auth_type = AuthType();
  std::string timestamp = CurrentTime();
  std::string hash_info = MD5util::GetMD516(id + mac + timestamp);

  // JSON
  cJSON *root = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(id.c_str()));
  cJSON_AddItemToObject(root, "type", cJSON_CreateNumber(req_auth_type));
  cJSON_AddItemToObject(root, "hashInfo",
                        cJSON_CreateString(hash_info.c_str()));
  cJSON_AddItemToObject(root, "timestamp",
                        cJSON_CreateString(timestamp.c_str()));

  std::string request_body = cJSON_PrintUnformatted(root);

  cJSON_Delete(root);

  return request_body;
}

bool AuthManager::ParseResponseBody(std::string response) {
  cJSON *root = NULL;
  cJSON *result = NULL;
  cJSON *token = NULL;
  cJSON *token_valid_time = NULL;
  cJSON *key = NULL;
  cJSON *key_valid_time = NULL;
  cJSON *timestamp = NULL;

  root = cJSON_Parse(response.c_str());

  result = cJSON_GetObjectItem(root, "result");
  int response_result = result->valueint;

  if (response_result != 0) {
    cJSON_Delete(root);
    LOG(ERROR) << "Post return failed";
    return false;
  }

  token = cJSON_GetObjectItem(root, "token");  // in base64
  token_valid_time = cJSON_GetObjectItem(root, "tokenvalidtime");
  key = cJSON_GetObjectItem(root, "key");  // in base64
  key_valid_time = cJSON_GetObjectItem(root, "keyvalidtime");
  timestamp = cJSON_GetObjectItem(root, "timestamp");

  // decypt
  // get keys from MD5
  std::string id = device_info_.GetDeviceId();
  std::string mac = device_info_.GetMac();

  std::string key_token =
      MD5util::GetMD516(id + mac + std::string(token_valid_time->valuestring));
  std::string key_key =
      MD5util::GetMD516(id + mac + key_valid_time->valuestring);

  // get text from base64
  std::string encypted_token = Base64util::ParseBase64(token->valuestring);
  std::string encypted_key = Base64util::ParseBase64(key->valuestring);

  // aes decypt
  auth_para_.SetToken(AESECButil::DecryptPKCS5(encypted_token, key_token));
  auth_para_.SetTokenValidTime(token_valid_time->valuestring);
  auth_para_.SetKey(AESECButil::DecryptPKCS5(encypted_key, key_key));
  auth_para_.SetKeyValidTime(key_valid_time->valuestring);

  cJSON_Delete(root);  // only need to delete root
  return true;
}

int AuthManager::AuthType() {
  switch (auth_status_) {
    case (int)(AUTH_STATUS::INVALID_ALL):
      return 0;
    case (int)(AUTH_STATUS::INVALID_TOKEN):
      return 1;
    case (int)(AUTH_STATUS::INVALID_KEY):
      return 2;
    default:
      break;
  }

  return 0;
}

void AuthManager::DoDeviceAuth() {
  if (DeviceAuth()) {
    if (CheckKeyValid()) {
      if (!CheckTokenValid()) {
        LOG(INFO) << "Valid key && Invalid token";
        SetAuthStatus(AUTH_STATUS::INVALID_TOKEN);
      } else {
        LOG(INFO) << "Valid key && Valid token";
        SetAuthStatus(AUTH_STATUS::AUTHED);
      }
    } else {
      LOG(INFO) << "Invalid key";
      if (!CheckTokenValid()) {
        LOG(INFO) << "Invalid token && Invalid key";
        SetAuthStatus(AUTH_STATUS::INVALID_ALL);
      }
      SetAuthStatus(AUTH_STATUS::INVALID_KEY);
    }
  } else {
    // set_auth_status(AUTH_STATUS::AUTHED);
    LOG(INFO) << "because not gettinng the correct result from server, "
                 "keep the auth status NONE";
  }
}

void AuthManager::AuthProc() {
  AUTH_STATUS auth_status;
  while (1) {
    auth_status = GetAuthStatus();
    if (auth_status == AUTH_STATUS::NONE) {
      // DeviceRegister();
      DoDeviceAuth();
    } else if (auth_status == AUTH_STATUS::AUTHED) {
      time_t time_to_key = SecFromValidTime(GetKeyValidTime());
      time_t time_to_token = SecFromValidTime(GetTokenValidTime());

      if (time_to_key < VALID_TIME_INTERVAL) {
        DoDeviceAuth();
        sleep(1);
      } else if (time_to_token < VALID_TIME_INTERVAL) {
        DoDeviceAuth();
        sleep(1);
      } else {
        time_to_key < time_to_token
            ? sleep(time_to_key - VALID_TIME_INTERVAL)
            : sleep(time_to_token - VALID_TIME_INTERVAL);
      }

      // TODO before the authentication is expired, renew it
    } else if (auth_status == AUTH_STATUS::INVALID_ALL ||
               auth_status == AUTH_STATUS::INVALID_KEY ||
               auth_status == AUTH_STATUS::INVALID_TOKEN) {
      DoDeviceAuth();
    }
  }
}
}  // namespace v2x
