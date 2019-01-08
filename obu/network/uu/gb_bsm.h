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
 * @file gb_bsm.h
 *
 * @brief BasicSafetyMessage in GB, MotionConfidenceSet and
 * VehicleSafetyExtensions excluded.
 */

#pragma once

#ifndef GB_BSM_H_
#define GB_BSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "message_set_common.h"

// used to fill the bsm message, now the parameter is all needed
typedef struct BsmParam {
  int32_t msgCnt;
  uint8_t id[8];
  int32_t milli_second;
  int32_t pos_lon;
  int32_t pos_lat;
  int32_t accuracy_pos;
  int32_t transmission;
  int32_t speed;
  int32_t heading;
  int32_t acc_lon;
  int32_t acc_lat;
  int32_t acc_ver;
  int32_t acc_yaw;
  int32_t size_width;
  int32_t size_length;
  int32_t vehicle_class;
  uint8_t token[4];
} BsmParam_t;

/**
 * @brief Create BSM frame.
 * @param ppBsm: second rank pointer to the BasicSafetyMessage structure
 *        pBsmCfg: pointer to the BsmConfig structure
 *        createOption: create option
 * @return 0 if success
 */
int GbBSMCreate(BasicSafetyMessage_t **ppBsm, const BsmConfig_t *pBsmCfg,
                GBCreateOption_t createOption);

/**
 * @brief Fill BSM frame.
 * @param ppBsm: second rank pointer to the BasicSafetyMessage structure
 *        pBsmCfg: pointer to the BsmConfig structure
 *        pBsmPara: pointer to the BsmParam structure
 * @return 0 if success
 */
int GbBSMFill(BasicSafetyMessage_t **ppBsm, const BsmConfig_t *pBsmCfg,
              const BsmParam_t *pBsmPara);
/**
 * @brief Encode BSM frame.
 * @param pBsm: pointer to the BasicSafetyMessage structure
 *        ppBuf: second rank pointer to the buffer
 *        bufSize: size of the buffer
 *        pCodeSize: pointer to size of the code encoded
 * @return 0 if success
 */
int GbBSMEncode(BasicSafetyMessage_t *pBsm, uint8_t **ppBuf, const int bufSize,
                int *pCodeSize);

/**
 * @brief Free BSM frame.
 * @param pBsm: pointer to the BasicSafetyMessage structure
 *        freeOption: free option
 * @return 0 if success
 */
int GbBSMFree(BasicSafetyMessage_t *ppBsm, const GBFreeOption_t freeOption);

#ifdef __cplusplus
}
#endif

#endif  // GB_BSM_H_
