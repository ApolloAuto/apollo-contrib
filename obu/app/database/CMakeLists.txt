cmake_minimum_required(VERSION 2.8)

aux_source_directory(. DIRSRCS)
add_library(v2x_db ${DIRSRCS})
add_dependencies(v2x_db proto)
