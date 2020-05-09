cmake_minimum_required(VERSION 3.0)

project(StereoCameraDemo)

include_directories("../../include")

message("Current folder is: ${CMAKE_CURRENT_LIST_DIR}")

if (WIN32)
  # set stuff for windows
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Only x64 is supported!\n please use '-G 'Visual Studio 15 2017 Win64'' as argument")
    endif()
else()
  # set stuff for other systems
    add_definitions(-std=c++11)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../dlib")
else()
    set(BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/../../lib")
endif()

link_directories("${BIN_DIR}")
link_libraries(StereoCamera ImageUtils)
############################################################
# Create an executable
############################################################

# Add an executable with the above sources
add_executable(StereoCameraDemo
    main.cpp mycamerahandler.cpp
)

set_target_properties(StereoCameraDemo
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)
