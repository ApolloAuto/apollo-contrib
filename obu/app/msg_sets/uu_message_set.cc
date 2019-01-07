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

/**uu_message_set
 * @file
 *
 * @brief LTE-V message set for platform.
 */

#include "uu_message_set.h"

namespace v2x {

int UuMessageSet::CreateBSM() {
  present_ = MessageFrame_PR_bsmFrame;
  int ret = 0;
  ret = GbBSMCreate(&uu_bsm_, pBsmCfg_, GB_CREAT_ALL);

  return ret;
}

int UuMessageSet::EncodeBSM(uint8_t **ppBuf, const int bufSize,
                              int *pCodeSize) {
  int ret = 0;

  ret = GbBSMFill(&uu_bsm_, pBsmCfg_, pBsmPara_);
  if (ret != 0) {
    return ret;
  }

  ret = GbBSMEncode(uu_bsm_, ppBuf, bufSize, pCodeSize);

  return ret;
}

int UuMessageSet::FreeBSM(const GBFreeOption_t freeOption) {
  int ret = 0;
  ret = GbBSMFree(uu_bsm_, freeOption);

  return ret;
}

int UuMessageSet::SetBSMCfg(BsmConfig_t *pBsmCfg) {
  if (pBsmCfg == nullptr) {
    return ERR_INPUT_NULL;
  }

  pBsmCfg_ = pBsmCfg;

  return 0;
}
int UuMessageSet::SetBSMPara(BsmParam_t *pBsmPara) {
  if (pBsmPara == nullptr) {
    return ERR_INPUT_NULL;
  }

  pBsmPara_ = pBsmPara;

  return 0;
}
}  // namespace v2x
