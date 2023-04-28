cmake_minimum_required(VERSION 3.23)
project(project3_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project3 STATIC
        include/api/console.cpp include/api/input.cpp include/api/core.cpp
        include/api/timer.cpp include/api/timer.cpp include/api/keypress_handler.cpp

        include/render/core.cpp include/render/tinyphysicsengine.cpp

        include/audio/core.cpp include/audio/audiochannel.cpp include/audio/audiointerface.cpp
        include/audio/source_types/audiosource_single.cpp include/audio/source_types/audiosource_circular.cpp
        include/audio/source_types/audiosource_looping.cpp include/audio/source_types/iaudiosource.cpp

        include/ui/strided_memcpy.cpp include/api/resource_locator.cpp include/ui/core.cpp)

target_include_directories(project3 PUBLIC include)
target_compile_definitions(project3 PUBLIC -DCOMPILER_DEBUG=1)

set(TRACY_ENABLE ON)
set(TRACY_CALLSTACK ON)
set(TRACY_STATIC ON)
add_subdirectory(tracy)

add_executable(project3_test driver.cpp include/resources.o)
target_link_libraries(project3_test PUBLIC project3 xaudio2_8)
target_link_libraries(project3_test PUBLIC Tracy::TracyClient)


add_executable(uigen uigen/driver.cpp uigen/core/fileloader.cpp uigen/parser/filecontext.cpp
        uigen/parser/filesystem.cpp uigen/parser/globalcontext.cpp uigen/parser/parsercore.cpp uigen/parser/parserconstruct.cpp uigen/cli.cpp)
target_include_directories(uigen PUBLIC uigen)