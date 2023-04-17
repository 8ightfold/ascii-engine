cmake_minimum_required(VERSION 3.23)
project(project3_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project3 STATIC
        include/api/console.cpp include/api/input.cpp include/api/core.cpp
        include/api/timer.cpp include/api/timer.cpp include/api/keypress_handler.cpp

        include/ui/strided_memcpy.cpp

        include/audio/core.cpp include/audio/audiochannel.cpp include/audio/audiointerface.cpp)

target_include_directories(project3 PUBLIC include)
target_compile_definitions(project3 PUBLIC -DCOMPILER_DEBUG=1)

add_executable(project3_test driver.cpp include/resources.o)
target_link_libraries(project3_test PUBLIC project3 xaudio2_8)