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

#include "message_set.h"
#include "glog/logging.h"

namespace v2x {

/**
 * @class MessageSet
 * @brief Provides 2 kinds of message frame: SPAT and MAP.
 * A message frame will be create and then decode from asn.1 code.
 */

MessageSet::MessageSet()
    : message_frame_(nullptr),map_(nullptr), spat_(nullptr), present_(0) {}

MessageSet::~MessageSet() {
  FreeAll();
}

int MessageSet::CreateMap() {
  //    reset();

  present_ = MessageFrame_PR_mapFrame;
  // using ctfo
  int ret = 0;
  ret = cv2x_message_frame_create(&message_frame_);

  // get MapData_t pointer from messageFrame created by ctfo
  map_ = &(message_frame_->choice.mapFrame);

  return ret;
}

int MessageSet::CreateSpat() {
  //   reset();

  present_ = MessageFrame_PR_spatFrame;
  // using ctfo
  int ret = 0;
  ret = cv2x_message_frame_create(&message_frame_);

  // get SPAT_t pointer from messageFrame created by ctfo
  spat_ = &(message_frame_->choice.spatFrame);

  return ret;
}

int MessageSet::DecodeMap(char* buf, const eCodeType code_type,
                           const int buf_size) {
  // using ctfo
  int ret = 0;
  ret = cv2x_message_frame_decode(&message_frame_, code_type, buf, buf_size);

  // for test
  PrintMessageFrame();

  return ret;
}

int MessageSet::DecodeSpat(char* buf, const eCodeType code_type,
                            const int buf_size) {
  // using ctfo
  int ret = 0;
  ret = cv2x_message_frame_decode(&message_frame_, code_type, buf, buf_size);

  // for test
  PrintMessageFrame();

  return ret;
}

MapData_t* MessageSet::GetMap() {
  if (present_ == MessageFrame_PR_mapFrame) {
    return map_;
  } else {
    return nullptr;
  }
}

SPAT_t* MessageSet::GetSpat() {
  if (present_ == MessageFrame_PR_spatFrame) {
    return spat_;
  } else {
    return nullptr;
  }
}

void MessageSet::FreeAll() {
  // using ctfo
  int ret = 0;
  if (message_frame_ != nullptr) {
    ret = cv2x_message_frame_free(message_frame_, 0);
  }
  if (ret != 0) {
    LOG(ERROR) << "free message failed(" << ret << ")";
  }
  return;
}

int MessageSet::Reset() {
  present_ = 0;

  int ret = 0;
  FreeAll();

  // if (ret == 0) {
  message_frame_ = nullptr;
  map_ = nullptr;
  spat_ = nullptr;
  // }

  return ret;
}

void MessageSet::PrintMessageFrame() {
  //   xer_fprint(stdout, &asn_DEF_MessageFrame, _messageFrame);
}

}  // namespace v2x
