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

#ifndef APP_MSG_SETS_UU_MESSAGE_SET_H
#define APP_MSG_SETS_UU_MESSAGE_SET_H

#include "vendor/cmcc/gb_bsm.h"

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {

/**
 * @class UuMessageSet
 *
 * @brief message set for platform
 */
class UuMessageSet {
 public:
  /**
   * @brief constructor
   */
  UuMessageSet()
      : present_(0),
        uu_bsm_(nullptr),
        pBsmPara_(nullptr),
        pBsmCfg_(nullptr) {}

  /**
   * @brief destructor
   */
  ~UuMessageSet() {
    FreeBSM(GB_FREE_ALL);
  }

  /**
   * @brief Create BSM frame.
   * @param void
   * @return 0 if success
   */
  int CreateBSM();

  /**
   * @brief Encode BSM frame.
   * @param ppBuf: second rank pointer to the buffer
   *        bufSize: size of the buffer
   *        pCodeSize: pointer to size of the code encoded
   * @return 0 if success
   */
  int EncodeBSM(uint8_t **ppBuf, const int bufSize, int *pCodeSize);

  /**
   * @brief Free BSM frame.
   * @param freeOption: free option
   * @return 0 if success
   */
  int FreeBSM(const GBFreeOption_t freeOption);

  /**
   * @brief Set bsm configuration data.
   * @param pBsmCfg: pointer to the bsm configuration structure
   * @return 0 if success
   */
  int SetBSMCfg(BsmConfig_t *pBsmCfg);

  /**
   * @brief Set bsm parameter to fill in the BSM message.
   * @param pBsmCfg: pointer to the bsm parameter structure
   * @return 0 if success
   */
  int SetBSMPara(BsmParam_t *pBsmPara);

 private:
  /**
   * @brief constructor
   */
  UuMessageSet(const UuMessageSet &msg_set);

  /**
   * @brief operator=
   */
  void operator=(const UuMessageSet &msg_set);
  int present_;
  BasicSafetyMessage_t *uu_bsm_;
  BsmParam_t *pBsmPara_;
  BsmConfig_t *pBsmCfg_;
};
}  // namespace v2x

#endif /* APP_MSG_SETS_UU_MESSAGE_SET_H */
