# cmake_minimum_required(VERSION 3.10)
# project(outman VERSION 1.0)
# 
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
# 
# include_directories(include)
# 
# file(GLOB_RECURSE SOURCES "include/*.tpp")
# 
# add_library(outman SHARED ${SOURCES})
# 
# install(TARGETS outman
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     RUNTIME DESTINATION bin
# )
# 
# install(DIRECTORY include/outman
#     DESTINATION include
#     FILES_MATCHING PATTERN "*.hpp"
# )
# 
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../out)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../out)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../out) 
