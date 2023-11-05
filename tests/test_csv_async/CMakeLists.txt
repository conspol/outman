cmake_minimum_required(VERSION 3.14)

project(csv_async_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../outman/include)

add_executable(csv_async_app main.cpp)

target_link_libraries(csv_async_app PRIVATE "stdc++fs" pthread)

# For VS Profiler 
# SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")

# add_custom_target(graphviz ALL
#                  "${CMAKE_COMMAND}" "--graphviz=graphviz_csv_async" .
#                  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
