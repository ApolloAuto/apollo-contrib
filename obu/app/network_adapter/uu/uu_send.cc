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
 * @file uu_send.cc
 * @brief adaptation module send the data to remote server via uu interface.
 */

#include "uu_send.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <string>
#include "cv2x_app_layer/cv2x_message_frame.h"
#include "cv2x_app.h"
#include "database/v2x_db.h"
#include "glog/logging.h"
#include "msg_sets/uu_message_set.h"
#include "network_adapter/pc5/adapter_pc5.h"
#include "platform_auth/auth_manager.h"
#include "platform_auth/encrypt_utils.h"
#include "coordinate_transition/coordinate_transition.h"
#include "proxy/proxy.h"
namespace v2x {

UuSend::UuSend(std::string src_addr, uint32_t src_port, uint32_t interval)
    : OsThread("UuSendThread", OS_THREAD_PRIORITY_NORMAL,
               OS_THREAD_DEFAULT_STACK_SIZE, true),
      src_addr_(src_addr),
      src_port_(src_port),
      interval_(interval) {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, 0);
  if (timerfd < 0) {
    LOG(ERROR) << "Failed in timerfd_create";
    return;
  }
  timer_fd_ = timerfd;
  is_cyclical_ = true;
  Initialize();
}

UuSend::~UuSend(void) {
  // close socket
  close(send_socket_);
  // close timer
  close(timer_fd_);
}

void UuSend::Initialize() {
  send_addr_.sin_family = AF_INET;
  send_addr_.sin_addr.s_addr = inet_addr(src_addr_.c_str());
  send_addr_.sin_port = htons(src_port_);

  send_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (send_socket_ < 0) {
    LOG(ERROR) << "create sending socket error!";
    return;
  }

  // struct timeval tv_out;
  // tv_out.tv_sec = 1;
  // tv_out.tv_usec = 0;
  // setsockopt(send_socket_, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
  return;
}
int UuSend::GetSocketFd() {
  return send_socket_;
}
bool UuSend::SendData(char *buf, uint32_t len) {
  int slen = sendto(send_socket_, buf, len, 0, (struct sockaddr *)&send_addr_,
                    sizeof(send_addr_));
  if (slen < 0) {
    LOG(ERROR) << "send got error: " << slen << ", " << strerror(errno);
    return false;
  }
  return true;
}

void UuSend::TimerExpired() {
  BaseAuthManager *auth_manager = BaseAuthManagerSingleton::Instance();
  uint32_t len = GetNetworkMtu();
  std::shared_ptr<UuMessageSet> message_set =
      std::shared_ptr<UuMessageSet>(new UuMessageSet());
  V2xProxy *grpc = V2xProxySingleton::Instance();
  // get position from carstatus
  auto car_status = grpc->GetCarStatus();
  if (car_status == nullptr) {
    LOG(INFO) << "can't get car status";
    if (!v2x::fLB::FLAGS_uu_debug) {
      return;
    }
  }

  BsmConfig_t bsm_config;
  memset(&bsm_config, 0x00, sizeof(BsmConfig_t));
  bsm_config.token = 4;

  message_set->SetBSMCfg(&bsm_config);

  BsmParam_t *bsm_para = new BsmParam_t;
  memset(bsm_para, 0x00, sizeof(BsmParam_t));

  std::string device_id = auth_manager->GetDeviceId();

  uint8_t id[8];
  uint32_t i;

  for (i = 0; i < (device_id.length() / 2); i++) {
    sscanf(device_id.c_str() + (2 * i), "%2hhx", id + i);
  }

  memcpy(&(bsm_para->id), id, 8);
  message_set->SetBSMPara(bsm_para);

  static int32_t msg_cnt = 0;
  if (msg_cnt == 127) {
    msg_cnt = 0;
  }
  bsm_para->msgCnt = msg_cnt++;

  double position_x = 0.0;
  double position_y = 0.0;

  if (!v2x::fLB::FLAGS_uu_debug) {
    position_x = car_status->localization().pose().position().x();
    position_y = car_status->localization().pose().position().y();
  } else {
    // this position is in Haidianjiaxiao
    position_x = 423915.980284080;
    position_y = 4437985.401765677;
  }
  LOG(INFO) << "Position x:y=" << std::fixed << std::setprecision(10) << position_x<<", " << position_y;
  double lat = 0;
  double lon = 0;

  CoordinateTransition coord_trans;
  coord_trans.UTMXYToLatLon(position_x, position_y, lat, lon);
  if (v2x::fLB::FLAGS_uu_debug) {
    // for test, real position in Wuxi
    lat = 31.4825152;
    lon = 120.319680;
  }
  LOG(INFO) << "Position x:y=" << std::fixed << std::setprecision(10) << lat<<", " << lon;
  bsm_para->pos_lat = static_cast<int32_t>(lat * 10000000);
  bsm_para->pos_lon = static_cast<int32_t>(lon * 10000000);
  LOG(INFO) << "BSM parameter, " << bsm_para->pos_lat << ", " << bsm_para->pos_lon;
  std::string auth_token = auth_manager->GetToken();

  uint8_t token[4];
  for (i = 0; i < (auth_token.length() / 2); i++) {
    sscanf(auth_token.c_str() + (2 * i), "%2hhx", token + i);
  }
  memcpy(&(bsm_para->token), token, 4);

  int encode_size;
  uint8_t *pbuf = nullptr;
  try {
    pbuf = new uint8_t[len];
  } catch (const std::bad_alloc &e) {
    LOG(ERROR) << "memory malloc failed for bsm_encrypt.";
    delete bsm_para;
    return;
  }

  message_set->EncodeBSM(&pbuf, len, &encode_size);
  if (encode_size < 0) {
    LOG(ERROR) << "encode failed.";
    delete[] pbuf;
    delete bsm_para;
    return;
  }
  LOG(INFO) << "encode size." << encode_size;
  // encypt
  char *bsm_encrypt = nullptr;
  // PKCS5 padding is lesss than 16bytes.
  try {
    bsm_encrypt = new char[encode_size + 16];
  } catch (const std::bad_alloc &e) {
    LOG(ERROR) << "memory malloc failed for bsm_encrypt.";
    delete[] pbuf;
    delete bsm_para;
    return;
  }
  memset(bsm_encrypt, 0x00, encode_size + 16);

  int bsm_encrypt_len = AESECButil::EncryptPKCS5(
      reinterpret_cast<unsigned char *>(pbuf), encode_size,
      reinterpret_cast<unsigned char *>(bsm_encrypt), auth_manager->GetKey());

  if (bsm_encrypt_len > 0) {
    if (!SendData(bsm_encrypt, bsm_encrypt_len)) {
      LOG(ERROR) << "send data failed.";
    }
  } else {
    LOG(ERROR) << "encrypt failed.";
  }
  delete[] pbuf;
  delete[] bsm_encrypt;
  delete bsm_para;
  return;
}

void UuSend::Run() {
  struct itimerspec timer_interval = {0};
  timer_interval.it_value.tv_sec = interval_ / 1000;
  timer_interval.it_value.tv_nsec = (interval_ % 1000) * 1000000LLU;

  timer_interval.it_interval.tv_sec = timer_interval.it_value.tv_sec;
  timer_interval.it_interval.tv_nsec = timer_interval.it_value.tv_nsec;
  BaseAuthManager *auth_manager = BaseAuthManagerSingleton::Instance();
  AUTH_STATUS auth_status = auth_manager->GetAuthStatus();
  while (auth_status != AUTH_STATUS::AUTHED) {
    sleep(1);
    auth_status = auth_manager->GetAuthStatus();
  }
  LOG(INFO) << "auth passed.";
  if (timerfd_settime(timer_fd_, 0, &timer_interval, NULL) == -1) {
    LOG(ERROR) << "timerfd_settime failed " << strerror(errno);
    return;
  }

  uint64_t howmany = 0;
  ssize_t size = 0;

  while (1) {
    size = ::read(timer_fd_, &howmany, sizeof(howmany));
    if (size != sizeof(howmany)) {
      LOG(ERROR) << "Periodical timer reads " << size << "bytes";
    }

    TimerExpired();
  }

  return;
}
}  // namespace v2x
