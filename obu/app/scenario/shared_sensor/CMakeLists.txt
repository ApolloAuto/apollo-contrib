cmake_minimum_required(VERSION 2.8)
aux_source_directory(. DIRSRCS)
add_library(ssm ${DIRSRCS})
add_dependencies(ssm proto policy)
