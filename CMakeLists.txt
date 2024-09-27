cmake_minimum_required(VERSION 3.16)
project(coro)

set(CMAKE_CXX_STANDARD 23)
set(SOURCES Main.cpp Executor.cpp Fence.cpp)
add_executable(coro ${SOURCES})