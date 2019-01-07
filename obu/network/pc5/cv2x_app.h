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
 * @file cv2x_app.h
 * @brief define network layer interface for application layer 
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAC_ADDR_LEN (6)
#define DSM_HDR_EXTS_LEN_MAX (256)  // 256

// C-V2X system interface start-----------------------

typedef enum _eMsgType_ {
  MSG_TYPE_BSM = 0,
  MSG_TYPE_RSI,
  MSG_TYPE_RSM,
  MSG_TYPE_MAP,
  MSG_TYPE_SPAT,
  MSG_TYPE_MAX
} eMsgType;

typedef struct _tDSMRxParm_ {
  uint8_t dsmpVer;
  uint32_t appId;
  char peerMac[MAC_ADDR_LEN];
  uint8_t priority;
} __attribute__((packed)) tDSMRxParm;

typedef struct _tDSMTxParm_ {
  uint32_t appId;                         // aid
  uint8_t protTp;                         // eProtTp value
  uint8_t priority;                       //
  char peerMac[MAC_ADDR_LEN];             //
  char dsmHdrExts[DSM_HDR_EXTS_LEN_MAX];  // must 0
} __attribute__((packed)) tDSMTxParm;

/**
 *
 * @brief: C-V2X network app system interface read function with blocking.
 *
 *
 */ 
int32_t v2x_system_read(char* const rxBuf, const int32_t bufLen,
                               tDSMRxParm* const rxDSMParm);

/**
 *
 * @brief: C-V2X network app system interface write function.
 *
 *
 */
int32_t v2x_system_write(char* const txBuf, const int32_t txLen,
                                tDSMTxParm* const txDSMParm);

/**
 *
 * @brief: C-V2X network app system interface init.
 *
 *
 */
extern int32_t v2x_system_init(void);

/**
 *
 * @brief: C-V2X network app system interface exit.
 *
 *
 */

extern int32_t v2x_system_exit(void);

/**
 *
 * @brief: Get the maximum transmission unit which is supported by network  * layer.
 *
 */
extern int32_t get_mtu(void);

/**
 *
 * @brief: Get the message type via appid paremeter.
 *
 */
extern eMsgType get_msg_type(const uint32_t aicAid);

#ifdef __cplusplus
}
#endif

