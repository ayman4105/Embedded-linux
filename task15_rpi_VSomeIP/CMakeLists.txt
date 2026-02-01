cmake_minimum_required(VERSION 3.18)
project(VSomeIP_Task15)

set(CMAKE_CXX_STANDARD 17)


find_package(vsomeip3 REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE vsomeip3)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE vsomeip3)
