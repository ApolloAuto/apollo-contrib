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

cmake_minimum_required(VERSION 2.8)
project(v2x)
# set the cross compiling
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER "arm-poky-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILER "arm-poky-linux-gnueabi-g++")
set(CMAKE_CXX_FLAGS "-std=c++11")
add_compile_options(-DDEBUG)
# where to find header files
include_directories(.)
include_directories(${v2x_SOURCE_DIR}/third_party/gflags/include)
include_directories(${v2x_SOURCE_DIR}/third_party/glog/include)
include_directories(${v2x_SOURCE_DIR}/third_party/grpc/include)
include_directories(${v2x_SOURCE_DIR}/third_party/protobuf/include)
#include_directories(/usr/local/include)
include_directories(${v2x_SOURCE_DIR}/third_party/obu/include)
include_directories(${v2x_SOURCE_DIR}/third_party/openssl/include)
include_directories(${v2x_SOURCE_DIR}/third_party/obu/include/asn1_source)
include_directories(${v2x_SOURCE_DIR}/third_party/security/include)
include_directories(${v2x_SOURCE_DIR}/third_party/curl/include)
include_directories(${v2x_SOURCE_DIR}/third_party/cJSON/include)
include_directories(${v2x_SOURCE_DIR}/third_party)
include_directories(${v2x_SOURCE_DIR})
include_directories(${v2x_SOURCE_DIR}/app)
include_directories(${v2x_SOURCE_DIR}/utils)
include_directories(${v2x_SOURCE_DIR}/network/pc5)
include_directories(${v2x_SOURCE_DIR}/security)
include_directories(${v2x_SOURCE_DIR}/vendor)
# where to find library
link_directories(${v2x_SOURCE_DIR}/third_party/glog/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/gflags/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/grpc/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/protobuf/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/obu/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/openssl/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/security/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/curl/lib)
link_directories(${v2x_SOURCE_DIR}/third_party/cJSON/lib)
link_directories(${v2x_SOURCE_DIR}/vendor/ctfo)
link_directories(${v2x_SOURCE_DIR}/vendor/cmcc/lib)
# which subdirectory to compile
add_subdirectory(app)
add_subdirectory(utils)
add_subdirectory(security)
add_library(obu_conf v2x_gflags.cc)
add_executable(v2x main.cc)
target_link_libraries(v2x ntw_adapter v2x_db tl policy proxy msg_set uu_auth obu_conf cmcc curl JSON glog gpr grpc++ protobuf utils proto security cncrypto cert ssl crypto cV2xAppLyr cV2xNtwkLyr selfcomm event pthread config) 
