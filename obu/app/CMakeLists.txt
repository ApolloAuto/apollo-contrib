cmake_minimum_required(VERSION 2.8)

find_program(PROTOBUF_PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
set(ENV{PROTOC_FILEPATH} ${PROTOBUF_PROTOC_EXECUTABLE})
set(ENV{GRPC_CPP_PLUGIN_FILEPATH} ${GRPC_CPP_PLUGIN})

message(STATUS "Compiling protos...")
execute_process(
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/proto-apollo/compile-apollo-proto.bash
        RESULT_VARIABLE script_res
)
if (NOT ${script_res} EQUAL 0)
    message(FATAL "Failed to compiling protos")
endif ()

include_directories(proto-apollo)
file(GLOB_RECURSE pb_cc_apollo ./proto-apollo *.pb.cc)
add_library(proto
        ${pb_cc_apollo}
        )

add_subdirectory(database)
#add_subdirectory(proto)
add_subdirectory(network_adapter)
add_subdirectory(scenario)
add_subdirectory(proxy)
add_subdirectory(msg_sets)
add_subdirectory(policies)
add_subdirectory(platform_auth)

