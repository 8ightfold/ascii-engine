cmake_minimum_required(VERSION 3.23)
project(project3_test)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(klein)

add_library(project3 STATIC
        include/api/console.cpp include/api/input.cpp include/api/core.cpp
        include/api/timer.cpp include/api/timer.cpp

        include/ui/strided_memcpy.cpp)
target_include_directories(project3 PUBLIC include)
target_compile_definitions(project3 PUBLIC -DCOMPILER_DEBUG=1)

add_executable(project3_test driver.cpp)
target_link_libraries(project3_test PUBLIC project3 klein::klein)