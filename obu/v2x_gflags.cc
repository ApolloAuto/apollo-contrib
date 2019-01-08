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
 * File: v2x_gflags.cc
 * Brief: Global flags definition.
 */

#include "v2x_gflags.h"

namespace v2x {
DEFINE_string(obu_ip_addr, "192.168.10.123", "OBU IP address");
DEFINE_string(svb_ip_addr, "192.168.10.6", "SVB IP address");
DEFINE_bool(security_flag, false, "security enable flag");
DEFINE_string(root_ca, "", "Root CA File");
DEFINE_string(self_ca, "", "Self CA File");
DEFINE_string(auth_url, "http://112.25.66.162:8003/v2x-auth/v1/auth",
              "Authentication url for UU mode");
DEFINE_string(uu_device_id, "0066680081700001",
              "Device id which is registered in UU server");
DEFINE_string(uu_device_mac, "D89EF3821FB6", "Device MAC address");
DEFINE_string(uu_server_ip_addr, "112.25.66.161", "UU server IP address");
DEFINE_uint32(uu_socket_port, 28120, "UDP socket port");
DEFINE_uint32(uu_message_interval_in_ms, 1000, "BSM message interval");
DEFINE_bool(uu_debug, false, "uu interface debug flag");
DEFINE_int32(zone, 50, "designating the UTM zone");
DEFINE_bool(southernhemi_flag, false, "the southern hemisphere flag");
DEFINE_bool(debug_flag, true, "the debug print flag");

}  // namespace v2x
