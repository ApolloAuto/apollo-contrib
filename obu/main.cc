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
 * @file main.cc
 * @brief main function for OBU
 */

#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include "database/v2x_db.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "network_adapter/pc5/adapter_pc5.h"
#include "network_adapter/uu/adapter_uu.h"
#include "network_adapter/uu/uu_send.h"
#include "platform_auth/auth_manager.h"
#include "platform_auth/auth_para.h"
#include "platform_auth/device_info.h"
#include "proxy/proxy.h"
#include "scenario/traffic_light/traffic_light_thread.h"
#include "scenario/shared_sensor/shared_sensor_thread.h"
#include "v2x_gflags.h"

void log_init() {
  std::string path(PATH_MAX, '\0');

  // get the current path
  int cnt = readlink("/proc/self/exe", (char*)path.c_str(), path.size());
  int i = 0;

  // delete the exe name
  for (i = cnt; i >= 0; --i) {
    if (path[i] == '/') {
      path[i + 1] = '\0';
      break;
    }
  }

  std::string path2(path, 0, i);

  // append log folder to path
  path = path2 + "/log";

  if (access(path.c_str(), F_OK) != 0) {
    std::string cmd("mkdir ");
    cmd += path;
    system(cmd.c_str());
  }

  // init glog
  google::InitGoogleLogging("v2x");

  // output log immediately
  FLAGS_logbufsecs = 0;
  // set the log file to 100MB
  FLAGS_max_log_size = 1;

  FLAGS_stop_logging_if_full_disk = true;

  // log with level >=ERROR is output to stderr
  google::SetStderrLogging(google::GLOG_FATAL);

  // set the path for the log file
  std::string dest_dir = path + "/info";

  google::SetLogDestination(google::GLOG_INFO, dest_dir.c_str());
  dest_dir = path + "/warning";
  google::SetLogDestination(google::GLOG_WARNING, dest_dir.c_str());
  dest_dir = path + "/error";
  google::SetLogDestination(google::GLOG_ERROR, dest_dir.c_str());
  dest_dir = path + "/fatal";
  google::SetLogDestination(google::GLOG_FATAL, dest_dir.c_str());
}

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  log_init();

  std::string auth_url = v2x::fLS::FLAGS_auth_url;
  v2x::AuthPara* auth_parameter = new v2x::AuthPara(auth_url);

  const std::string device_id = v2x::fLS::FLAGS_uu_device_id;
  const std::string mac = v2x::fLS::FLAGS_uu_device_mac;
  const std::string phone_num = "";
  v2x::DeviceInfo* device_info = new v2x::DeviceInfo(device_id, mac, phone_num);

  v2x::BaseAuthManager* auth_manager = v2x::BaseAuthManager::Create(
      (int)v2x::PLATFORM_VENDOR::CMCC_V2X, *auth_parameter, *device_info);
  if (auth_manager == nullptr) {
    LOG(FATAL) << "Create the auth manager failed";
    return 1;
  }

  v2x::BaseAuthManagerSingleton::InstallInstance(auth_manager);
  auth_manager->Start();

  v2x::V2xDbSingleton::InstallInstance(new v2x::V2xDb());
  v2x::V2xDb* db = v2x::V2xDbSingleton::Instance();
  db->V2xDbInit();

  std::string data_url = v2x::fLS::FLAGS_uu_server_ip_addr;
  uint32_t data_port = v2x::fLU::FLAGS_uu_socket_port;
  v2x::UuSend* uu_send = new v2x::UuSend(
      data_url, data_port, v2x::fLU::FLAGS_uu_message_interval_in_ms);
  v2x::AdapterUu* adpt_uu = new v2x::AdapterUu(uu_send->GetSocketFd());

  uu_send->Start();
  adpt_uu->Start();

  std::string obu_address = v2x::fLS::FLAGS_obu_ip_addr + ":50100";
  std::string svb_address = v2x::fLS::FLAGS_svb_ip_addr + ":50101";
  v2x::V2xProxySingleton::InstallInstance(
      new v2x::V2xProxy(obu_address, svb_address));
  v2x::V2xProxy* grpc = v2x::V2xProxySingleton::Instance();
  grpc->StartAll();

  v2x::TrafficLightThread* traffic_light_thread = new v2x::TrafficLightThread();
  traffic_light_thread->Start();
  v2x::SharedSensorThread* shared_sensor_thread = new v2x::SharedSensorThread();
  shared_sensor_thread->Start();
  bool security_flag = v2x::fLB::FLAGS_security_flag;
  std::string root_ca = v2x::fLS::FLAGS_root_ca;
  std::string self_ca = v2x::fLS::FLAGS_self_ca;

  v2x::Adapter* recv_thread = new v2x::Adapter(security_flag, root_ca, self_ca);
  recv_thread->Start();
  recv_thread->Join();
  return 0;
}
