
cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(../../third_party/openssl/include/)
include_directories(../../third_party/cJSON/include/)
include_directories(../../third_party/curl/include/)
include_directories(../msg_sets)
include_directories(../../utils)

add_library(uu_auth encrypt_utils.cc auth_manager.cc)

