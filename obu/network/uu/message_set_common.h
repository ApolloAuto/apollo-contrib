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
 * @brief message set common utils, now just for BSM.
 */

#ifndef MESSAGE_SET_COMMON_H_
#define MESSAGE_SET_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "MessageFrame.h"

#define IS_NULL(arg) ((arg) == NULL)
#define ALLOC_OBJ(type, size, ptr) do { \
  ptr = (type *)calloc(1, size); \
} while(0)
// error code
#define CV2X_SUCCESS (0)
#define ERR_CV2X_CALLOC (6)
#define ERR_INPUT_NULL (1000)
#define ERR_CV2X_BSM_UPER_ENCODE (5006)

// define of the message config
typedef struct Position3DConfig {
  uint8_t elevation;
} Position3DConfig_t;

typedef struct PositionConfidenceSetConfig {
  uint8_t elevation;
} PositionConfidenceSetConfig_t;

typedef struct BrakeSystemStatusConfig {
  uint8_t brakePadel;
  uint8_t wheelBrakes;
  uint8_t traction;
  uint8_t abs;
  uint8_t scs;
  uint8_t brakeBoost;
  uint8_t auxBrakes;
} BrakeSystemStatusConfig_t;

typedef struct VehicleSizeConfig {
  uint8_t height;
} VehicleSizeConfig_t;

typedef struct BsmConfig {
  uint8_t plateNo_size;  // size
  Position3DConfig_t posCfg;
  PositionConfidenceSetConfig_t accuracyCfg;
  uint8_t angle;
  uint8_t motionCfd;
  // TODO
  // MotionConfidenceSetConfig_t motionCfdCfg;
  BrakeSystemStatusConfig_t brakesCfg;
  VehicleSizeConfig_t sizeCfg;
  uint8_t safetyExt;
  // TODO
  // VehicleSafetyExtensionsConfig_t safetyExtCfg;
  uint8_t token;
} BsmConfig_t;

typedef enum GBCreateOption {
  GB_CREAT_ALL = 0,
  GB_CREAT_CONTENTS_ONLY = 1
} GBCreateOption_t;

typedef enum GBFreeOption {
  GB_FREE_ALL = 0,
  GB_FREE_CONTENTS_ONLY = 1
} GBFreeOption_t;

#ifdef __cplusplus
}
#endif

#endif /* MESSAGE_SET_COMMON_H_ */
