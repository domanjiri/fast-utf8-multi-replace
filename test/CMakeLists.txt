cmake_minimum_required(VERSION 3.5)
 
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++2a -O3 -msse4.2")
    set(CMAKE_EXE_LINKER_FLAGS "-s")
endif()
  
add_library(replace SHARED ../replace.cpp)

add_executable(runTests replace_test.cpp)
target_link_libraries(runTests ${GTEST_LIBRARIES} replace pthread -ltbb)
