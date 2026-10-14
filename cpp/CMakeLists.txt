cmake_minimum_required(VERSION 3.27.4)
project(sandbox-c-hello-world)

set(CMAKE_CXX_STANDARD 20)

add_executable(sandbox-c-hello-world main.cpp)

add_executable(sandbox-modbus main_modbus.cpp)
target_link_libraries(sandbox-modbus modbus)
