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
 * File: v2x_gflags.h
 * Brief: Global flags definition.
 */

#ifndef V2X_COMMON_V2X_GFLAGS_H
#define V2X_COMMON_V2X_GFLAGS_H

#include "gflags/gflags.h"

namespace v2x {
DECLARE_string(obu_ip_addr);
DECLARE_string(svb_ip_addr);
DECLARE_bool(security_flag);
DECLARE_string(root_ca);
DECLARE_string(self_ca);
DECLARE_string(auth_url);
DECLARE_string(uu_device_id);
DECLARE_string(uu_device_mac);
DECLARE_string(uu_server_ip_addr);
DECLARE_uint32(uu_socket_port);
DECLARE_uint32(uu_message_interval_in_ms);
DECLARE_bool(uu_debug);
DECLARE_int32(zone);
DECLARE_bool(southernhemi_flag);
DECLARE_bool(debug_flag);

}  // namespace v2x

#endif  // V2X_COMMON_V2X_GFLAGS_H
