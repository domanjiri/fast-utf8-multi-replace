cmake_minimum_required(VERSION 3.5)

project (replace)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++2a -O3 -msse4.2")
    set(CMAKE_EXE_LINKER_FLAGS "-s")
endif()

add_executable(replace replace.cpp)
target_link_libraries(replace -pthread -ltbb)
