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

#ifndef APP_MSG_SETS_MESSAGESET_H
#define APP_MSG_SETS_MESSAGESET_H

#include "message_set_lib.h"
#include "stdio.h"

/**
 * @namespace v2x
 * @brief v2x
 */
namespace v2x {

/**
 * @class MessageSet
 * @brief Provides 2 kinds of message frame: SPAT and MAP.
 * A message frame will be create and then decode from asn.1 code.
 */

class MessageSet {
 public:
  /**
   * @brief constructor
   */
  MessageSet();

  /**
   * @brief destructor
   */
  ~MessageSet();

  /**
   * @brief Create a MAP message frame.
   * @param void
   * @return v2x error number, 0 is success.
   */
  int CreateMap();

  /**
   * @brief Create a SPAT message frame.
   * @param void
   * @return v2x error number, 0 is success.
   */
  int CreateSpat();

  /**
   * @brief Decode a MAP message frame from asn.1 code.
   * @param
   * @return v2x error number, 0 is success.
   */
  int DecodeMap(char *buf, const eCodeType code_type, const int buf_size);

  /**
   * @brief Decode a SPAT message frame from asn.1 code.
   * @param
   * @return v2x error number, 0 is success.
   */
  int DecodeSpat(char *buf, const eCodeType code_type, const int buf_size);

  /**
   * @brief Get a MAP message frame.
   * @param void
   * @return Pointer to MAP message frame.
   */
  MapData_t *GetMap();

  /**
   * @brief Get a SPAT message frame.
   * @param void
   * @return Pointer to SPAT message frame.
   */
  SPAT_t *GetSpat();

  /**
   * @brief Free the memory of message frame.
   * @param void
   * @return v2x error number, 0 is success.
   */
  void FreeAll();

 private:
  /**
   * @brief constructor
   */
  MessageSet(const MessageSet &messgeSet);

  /**
   * @brief operator=
   */
  void operator=(const MessageSet &messgeSet);
#if 0
    /**
      * @brief Free the memory of message frame.
      * @param void
      * @return v2x error number, 0 is success.
      */
    int free_all();
#endif
  /**
   * @brief Reset all the variable.
   * @param void
   * @return v2x error number, 0 is success.
   */
  int Reset();

  /**
   * @brief Print messageFrame.
   * @param void
   * @return void
   */
  void PrintMessageFrame();

  MessageFrame_t *message_frame_;
  MapData_t *map_;
  SPAT_t *spat_;
  int present_;
};

}  // namespace v2x

#endif /* MESSAGESET_H_ */
