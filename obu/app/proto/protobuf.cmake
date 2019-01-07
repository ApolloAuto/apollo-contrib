cmake_minimum_required(VERSION 2.8)

function(V2X_PROTOBUF_GENERATE_CPP PATH SRCS HDRS)
    if(NOT ARGN)
        message(SEND_ERROR "Error: V2X_PROTOBUF_GENERATE_CPP() called without any proto files")
        return()
    endif()

    if(PROTOBUF_GENERATE_CPP_APPEND_PATH) # This variable is common for all types of output.
        # Create an include path for each file specified
        foreach(FIL ${ARGN})
            get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
            get_filename_component(ABS_PATH ${ABS_FIL} PATH)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    else()
        set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(DEFINED PROTOBUF_IMPORT_DIRS)
        foreach(DIR ${PROTOBUF_IMPORT_DIRS})
            get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    endif()

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        list(APPEND ${SRCS} "${PATH}/${FIL_WE}.pb.cc")
        list(APPEND ${HDRS} "${PATH}/${FIL_WE}.pb.h")

        #execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PATH})
        
        add_custom_command(
            OUTPUT  "${PATH}/${FIL_WE}.pb.cc"
                    "${PATH}/${FIL_WE}.pb.h"
            # COMMAND  ${Protobuf_PROTOC_EXECUTABLE} # /bin/sh: 1: -I: not found
            COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
            ARGS --cpp_out=${PATH} ${_protobuf_include_path} ${FIL}
            DEPENDS ${ABS_FIL}
            COMMENT "Running C++ protocol buffer compiler on ${FIL}"
            VERBATIM )

    endforeach()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()


######
#grpc#
######
function(V2X_PROTOBUF_GENERATE_GRPC_CPP PATH SRCS HDRS)
    if(NOT ARGN)
        message(SEND_ERROR "Error: V2X_PROTOBUF_GENERATE_GRPC_CPP() called without any proto files")
        return()
    endif()

    if(PROTOBUF_GENERATE_CPP_APPEND_PATH) # This variable is common for all types of output.
        # Create an include path for each file specified
        foreach(FIL ${ARGN})
            get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
            get_filename_component(ABS_PATH ${ABS_FIL} PATH)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    else()
        set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(DEFINED PROTOBUF_IMPORT_DIRS)
        foreach(DIR ${PROTOBUF_IMPORT_DIRS})
            get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    endif()

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        list(APPEND ${SRCS} "${PATH}/${FIL_WE}.grpc.pb.cc")
        list(APPEND ${HDRS} "${PATH}/${FIL_WE}.grpc.pb.h")

        #execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PATH})
        
        add_custom_command(
            OUTPUT  "${PATH}/${FIL_WE}.grpc.pb.cc"
                    "${PATH}/${FIL_WE}.grpc.pb.h"
            # COMMAND  ${Protobuf_PROTOC_EXECUTABLE} # /bin/sh: 1: -I: not found
            COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
            ARGS --grpc_out=${PATH} --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} ${_protobuf_include_path} ${FIL}
            DEPENDS ${ABS_FIL}
            COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
            VERBATIM )
    endforeach()    

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

