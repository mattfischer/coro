cmake_minimum_required(VERSION 3.16)
project(coro)

set(CMAKE_CXX_STANDARD 23)
set(SOURCES Actor.cpp Executor.cpp Main.cpp Task.cpp)
add_executable(coro ${SOURCES})