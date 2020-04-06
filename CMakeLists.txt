cmake_minimum_required(VERSION 3.10)

project (replace)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++2a -O3 -msse4.2")
    set(CMAKE_EXE_LINKER_FLAGS "-s")
endif()

add_subdirectory(test)

add_library(replace SHARED replace.cpp)

add_executable(main main.cpp)
target_link_libraries(main replace -pthread -ltbb)

