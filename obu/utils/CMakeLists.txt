cmake_minimum_required(VERSION 2.8)

#add_subdirectory(coordinate_transition)
#aux_source_directory(. DIRSRCS)
add_library(utils os_thread.cc coordinate_transition/coordinate_transition.cc)

